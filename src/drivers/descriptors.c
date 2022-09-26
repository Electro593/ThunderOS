/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                         **
**  Author: Aria Seiler                                                    **
**                                                                         **
**  This program is in the public domain. There is no implied warranty,    **
**  so use it at your own risk.                                            **
**                                                                         **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifdef INCLUDE_HEADER

extern void InterruptSwitch(void);
extern u64 GetInterruptStep(void);

typedef struct tss {
   u32 _Reserved0;
   u32 RSP0L;
   u32 RSP0H;
   u32 RSP1L;
   u32 RSP1H;
   u32 RSP2L;
   u32 RSP2H;
   u64 _Reserved1;
   u32 IST1L;
   u32 IST1H;
   u32 IST2L;
   u32 IST2H;
   u32 IST3L;
   u32 IST3H;
   u32 IST4L;
   u32 IST4H;
   u32 IST5L;
   u32 IST5H;
   u32 IST6L;
   u32 IST6H;
   u32 IST7L;
   u32 IST7H;
   u64 _Reserved2;
   u16 _Reserved3;
   u16 IOMapOffset;
} __attribute__((packed)) tss;

typedef struct gdt_entry {
   u16 Limit;
   u16 BaseP1;
   u08 BaseP2;
   u08 AccessByte;
   u08 Attributes;
   u08 BaseP3;
} __attribute__((packed)) gdt_entry;

typedef struct gdt {
   gdt_entry Entries[5];
} __attribute__((packed)) gdt;

typedef struct idt_gate_descriptor {
   u16 OffsetP1;
   u16 SegmentSelector;
   u08 ISTOffset;
   u08 Attributes;
   u16 OffsetP2;
   u32 OffsetP3;
   u32 _Reserved;
} idt_gate_descriptor;

typedef struct idt {
   idt_gate_descriptor Entries[256];
} idt;

vptr InterruptHandlers[256];

#endif

#ifdef INCLUDE_SOURCE

internal void
GDT_Init(IN gdt *GDT, IN tss *TSS, vptr *RingStacks, vptr *ISTStacks)
{
   Mem_Set(GDT, 0, sizeof(gdt));
   
   GDT->Entries[1].AccessByte = 0b10011010;
   GDT->Entries[1].Attributes = 0b00100000;
   
   GDT->Entries[2].AccessByte = 0b10010010;
   GDT->Entries[2].Attributes = 0b00000000;
   
   GDT->Entries[3].Limit = sizeof(tss);
   GDT->Entries[3].AccessByte = 0b10001001;
   GDT->Entries[3].Attributes = 0b00010000;
   GDT->Entries[3].BaseP1 = (u64)TSS & 0xFFFF;
   GDT->Entries[3].BaseP2 = ((u64)TSS >> 16) & 0xFF;
   GDT->Entries[3].BaseP3 = ((u64)TSS >> 24) & 0xFF;
   u32 *BaseP4 = (u32*)(GDT->Entries+4);
   *BaseP4 = (u64)TSS >> 32;
   
   TSS->_Reserved0 = 0;
   TSS->RSP0L = (u64)RingStacks[0] & 0xFFFFFFFF;
   TSS->RSP0H = (u64)RingStacks[0] >> 32;
   TSS->RSP1L = (u64)RingStacks[1] & 0xFFFFFFFF;
   TSS->RSP1H = (u64)RingStacks[1] >> 32;
   TSS->RSP2L = (u64)RingStacks[2] & 0xFFFFFFFF;
   TSS->RSP2H = (u64)RingStacks[2] >> 32;
   TSS->_Reserved1 = 0;
   TSS->IST1L = ((u64)ISTStacks[0] + 0x1000) & 0xFFFFFFFF;
   TSS->IST1H = ((u64)ISTStacks[0] + 0x1000) >> 32;
   TSS->IST2L = ((u64)ISTStacks[1] + 0x1000) & 0xFFFFFFFF;
   TSS->IST2H = ((u64)ISTStacks[1] + 0x1000) >> 32;
   TSS->IST3L = ((u64)ISTStacks[2] + 0x1000) & 0xFFFFFFFF;
   TSS->IST3H = ((u64)ISTStacks[2] + 0x1000) >> 32;
   TSS->IST4L = ((u64)ISTStacks[3] + 0x1000) & 0xFFFFFFFF;
   TSS->IST4H = ((u64)ISTStacks[3] + 0x1000) >> 32;
   TSS->IST5L = ((u64)ISTStacks[4] + 0x1000) & 0xFFFFFFFF;
   TSS->IST5H = ((u64)ISTStacks[4] + 0x1000) >> 32;
   TSS->IST6L = ((u64)ISTStacks[5] + 0x1000) & 0xFFFFFFFF;
   TSS->IST6H = ((u64)ISTStacks[5] + 0x1000) >> 32;
   TSS->IST7L = ((u64)ISTStacks[6] + 0x1000) & 0xFFFFFFFF;
   TSS->IST7H = ((u64)ISTStacks[6] + 0x1000) >> 32;
   TSS->_Reserved2 = 0;
   TSS->_Reserved3 = 0;
   TSS->IOMapOffset = 0x0068;
   
   u08 *IOMap = (u08*)TSS + TSS->IOMapOffset;
   *(u16*)IOMap = 0xFFFF;
   
   SetGDTR(GDT, sizeof(gdt)-1);
}

internal void
IDT_SetEntry(idt *IDT, u32 Index, vptr Handler)
{
   InterruptHandlers[Index] = Handler;
   
   u64 Step = GetInterruptStep();
   u64 Address = (u64)InterruptSwitch + Step*Index;
   IDT->Entries[Index].OffsetP1 = (u16)(Address>> 0);
   IDT->Entries[Index].OffsetP2 = (u16)(Address>>16);
   IDT->Entries[Index].OffsetP3 = (u32)(Address>>32);
   IDT->Entries[Index].SegmentSelector = 0x08;
   IDT->Entries[Index].ISTOffset = 0;
   IDT->Entries[Index].Attributes = 0b10001110;
   IDT->Entries[Index]._Reserved = 0;
}

internal void
IDT_Init(IN idt *IDT)
{
   Mem_Set(InterruptHandlers, 0, sizeof(InterruptHandlers));
   
   IDT_SetEntry(IDT,   0, Exception_DivideByZero);
   IDT_SetEntry(IDT,   1, Exception_Debug);
   IDT_SetEntry(IDT,   2, Exception_NonMaskableInterrupt);
   IDT_SetEntry(IDT,   3, Exception_Breakpoint);
   IDT_SetEntry(IDT,   4, Exception_Overflow);
   IDT_SetEntry(IDT,   5, Exception_BoundsCheck);
   IDT_SetEntry(IDT,   6, Exception_InvalidOpcode);
   IDT_SetEntry(IDT,   7, Exception_DeviceNotAvailable);
   IDT_SetEntry(IDT,   8, Exception_DoubleFault);
   IDT_SetEntry(IDT,  10, Exception_InvalidTSS);
   IDT_SetEntry(IDT,  11, Exception_SegmentNotPresent);
   IDT_SetEntry(IDT,  12, Exception_StackSegmentFault);
   IDT_SetEntry(IDT,  13, Exception_GeneralProtection);
   IDT_SetEntry(IDT,  14, Exception_PageFault);
   IDT_SetEntry(IDT,  16, Exception_FloatingPoint);
   IDT_SetEntry(IDT,  17, Exception_AlignmentCheck);
   IDT_SetEntry(IDT,  18, Exception_MachineCheck);
   IDT_SetEntry(IDT,  19, Exception_SIMDFloatingPoint);
   IDT_SetEntry(IDT,  20, Exception_Virtualization);
   IDT_SetEntry(IDT,  21, Exception_ControlProtection);
   IDT_SetEntry(IDT,  28, Exception_HypervisorInjection);
   IDT_SetEntry(IDT,  29, Exception_VMMCommunication);
   IDT_SetEntry(IDT,  30, Exception_Security);
   
   SetIDTR(IDT, sizeof(idt)-1);
}

#undef EXCEPTIONS

#endif