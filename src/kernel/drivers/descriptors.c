/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                           **
**  Author: Aria Seiler                                                      **
**                                                                           **
**  This program is in the public domain. There is no implied warranty,      **
**  so use it at your own risk.                                              **
**                                                                           **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <util/mem.h>
#include <drivers/descriptors.h>
#include <drivers/interrupts.h>
#include <kernel/kernel.h>

vptr InterruptHandlers[256];

vptr InterruptSwitches[256] = {
#define N(Num) InterruptSwitch##Num,
	INTERRUPT_NUMS
#undef N
};

page ISTStacks[7];
page RingStacks[3];

gdt _GDT;
tss _TSS;

extern void SetSegments(u16 CS, u16 SS);
extern void SetTR(u16 GdtOffset);

void
GDT_Init(void)
{
	gdt *GDT = &_GDT;
	tss *TSS = &_TSS;

	Mem_Set(GDT, 0, sizeof(gdt));

	GDT->Entries[1].AccessByte = 0b10011010;
	GDT->Entries[1].Attributes = 0b00100000;

	GDT->Entries[2].AccessByte = 0b10010010;
	GDT->Entries[2].Attributes = 0b00000000;

	GDT->Entries[3].Limit	   = sizeof(tss);
	GDT->Entries[3].AccessByte = 0b10001001;
	GDT->Entries[3].Attributes = 0b00010000;
	GDT->Entries[3].BaseP1	   = (u64) TSS & 0xFFFF;
	GDT->Entries[3].BaseP2	   = ((u64) TSS >> 16) & 0xFF;
	GDT->Entries[3].BaseP3	   = ((u64) TSS >> 24) & 0xFF;
	u32 *BaseP4				   = (u32 *) (GDT->Entries + 4);
	*BaseP4					   = (u64) TSS >> 32;

	TSS->_Reserved0	 = 0;
	TSS->RSP0L		 = ((s64) &RingStacks[0] + 0x1000) & 0xFFFFFFFF;
	TSS->RSP0H		 = ((s64) &RingStacks[0] + 0x1000) >> 32;
	TSS->RSP1L		 = ((s64) &RingStacks[1] + 0x1000) & 0xFFFFFFFF;
	TSS->RSP1H		 = ((s64) &RingStacks[1] + 0x1000) >> 32;
	TSS->RSP2L		 = ((s64) &RingStacks[2] + 0x1000) & 0xFFFFFFFF;
	TSS->RSP2H		 = ((s64) &RingStacks[2] + 0x1000) >> 32;
	TSS->_Reserved1	 = 0;
	TSS->IST1L		 = ((s64) &ISTStacks[0] + 0x1000) & 0xFFFFFFFF;
	TSS->IST1H		 = ((s64) &ISTStacks[0] + 0x1000) >> 32;
	TSS->IST2L		 = ((s64) &ISTStacks[1] + 0x1000) & 0xFFFFFFFF;
	TSS->IST2H		 = ((s64) &ISTStacks[1] + 0x1000) >> 32;
	TSS->IST3L		 = ((s64) &ISTStacks[2] + 0x1000) & 0xFFFFFFFF;
	TSS->IST3H		 = ((s64) &ISTStacks[2] + 0x1000) >> 32;
	TSS->IST4L		 = ((s64) &ISTStacks[3] + 0x1000) & 0xFFFFFFFF;
	TSS->IST4H		 = ((s64) &ISTStacks[3] + 0x1000) >> 32;
	TSS->IST5L		 = ((s64) &ISTStacks[4] + 0x1000) & 0xFFFFFFFF;
	TSS->IST5H		 = ((s64) &ISTStacks[4] + 0x1000) >> 32;
	TSS->IST6L		 = ((s64) &ISTStacks[5] + 0x1000) & 0xFFFFFFFF;
	TSS->IST6H		 = ((s64) &ISTStacks[5] + 0x1000) >> 32;
	TSS->IST7L		 = ((s64) &ISTStacks[6] + 0x1000) & 0xFFFFFFFF;
	TSS->IST7H		 = ((s64) &ISTStacks[6] + 0x1000) >> 32;
	TSS->_Reserved2	 = 0;
	TSS->_Reserved3	 = 0;
	TSS->IOMapOffset = 0x0068;
	TSS->IOMap		 = 0xFFFF;

	SetGDTR(GDT, sizeof(gdt) - 1);

	SetSegments(0x08, 0x10);
	SetTR(0x18);
}

static void
IDT_SetEntry(idt *IDT, u32 Index, vptr Handler)
{
	InterruptHandlers[Index] = Handler;

	u64 Address							= (u64) InterruptSwitches[Index];
	IDT->Entries[Index].OffsetP1		= (u16) (Address >> 0);
	IDT->Entries[Index].OffsetP2		= (u16) (Address >> 16);
	IDT->Entries[Index].OffsetP3		= (u32) (Address >> 32);
	IDT->Entries[Index].SegmentSelector = 0x08;
	IDT->Entries[Index].ISTOffset		= 0b001;
	IDT->Entries[Index].Attributes		= 0b10001110;
	IDT->Entries[Index]._Reserved		= 0;
}

void
IDT_Init(idt *IDT)
{
	Mem_Set(InterruptHandlers, 0, sizeof(InterruptHandlers));

	IDT_SetEntry(IDT, 0, Exception_DivideByZero);
	IDT_SetEntry(IDT, 1, Exception_Debug);
	IDT_SetEntry(IDT, 2, Exception_NonMaskableInterrupt);
	IDT_SetEntry(IDT, 3, Exception_Breakpoint);
	IDT_SetEntry(IDT, 4, Exception_Overflow);
	IDT_SetEntry(IDT, 5, Exception_BoundsCheck);
	IDT_SetEntry(IDT, 6, Exception_InvalidOpcode);
	IDT_SetEntry(IDT, 7, Exception_DeviceNotAvailable);
	IDT_SetEntry(IDT, 8, Exception_DoubleFault);
	IDT_SetEntry(IDT, 10, Exception_InvalidTSS);
	IDT_SetEntry(IDT, 11, Exception_SegmentNotPresent);
	IDT_SetEntry(IDT, 12, Exception_StackSegmentFault);
	IDT_SetEntry(IDT, 13, Exception_GeneralProtection);
	IDT_SetEntry(IDT, 14, Exception_PageFault);
	IDT_SetEntry(IDT, 16, Exception_FloatingPoint);
	IDT_SetEntry(IDT, 17, Exception_AlignmentCheck);
	IDT_SetEntry(IDT, 18, Exception_MachineCheck);
	IDT_SetEntry(IDT, 19, Exception_SIMDFloatingPoint);
	IDT_SetEntry(IDT, 20, Exception_Virtualization);
	IDT_SetEntry(IDT, 21, Exception_ControlProtection);
	IDT_SetEntry(IDT, 28, Exception_HypervisorInjection);
	IDT_SetEntry(IDT, 29, Exception_VMMCommunication);
	IDT_SetEntry(IDT, 30, Exception_Security);

	SetIDTR(IDT, sizeof(idt) - 1);
}

#undef EXCEPTIONS
