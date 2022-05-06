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

extern vptr InterruptHandlers[];

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
   TSS->IST1L = (u64)ISTStacks[0] & 0xFFFFFFFF;
   TSS->IST1H = (u64)ISTStacks[0] >> 32;
   TSS->IST2L = (u64)ISTStacks[1] & 0xFFFFFFFF;
   TSS->IST2H = (u64)ISTStacks[1] >> 32;
   TSS->IST3L = (u64)ISTStacks[2] & 0xFFFFFFFF;
   TSS->IST3H = (u64)ISTStacks[2] >> 32;
   TSS->IST4L = (u64)ISTStacks[3] & 0xFFFFFFFF;
   TSS->IST4H = (u64)ISTStacks[3] >> 32;
   TSS->IST5L = (u64)ISTStacks[4] & 0xFFFFFFFF;
   TSS->IST5H = (u64)ISTStacks[4] >> 32;
   TSS->IST6L = (u64)ISTStacks[5] & 0xFFFFFFFF;
   TSS->IST6H = (u64)ISTStacks[5] >> 32;
   TSS->IST7L = (u64)ISTStacks[6] & 0xFFFFFFFF;
   TSS->IST7H = (u64)ISTStacks[6] >> 32;
   TSS->_Reserved2 = 0;
   TSS->_Reserved3 = 0;
   TSS->IOMapOffset = sizeof(tss);
   
   SetGDTR(GDT, sizeof(gdt)-1);
}

#define EXCEPTIONS \
   ELEM( 0, DivideByZero,         void) \
   ELEM( 1, Debug,                void) \
   ELEM( 2, NonMaskableInterrupt, void) \
   ELEM( 3, Breakpoint,           void) \
   ELEM( 4, Overflow,             void) \
   ELEM( 5, BoundsCheck,          void) \
   ELEM( 6, InvalidOpcode,        void) \
   ELEM( 7, DeviceNotAvailable,   void) \
   ELEM( 8, DoubleFault,          u32 ErrorCode) \
   ELEM(10, InvalidTSS,           u32 ErrorCode) \
   ELEM(11, SegmentNotPresent,    u32 ErrorCode) \
   ELEM(12, StackSegmentFault,    u32 ErrorCode) \
   ELEM(13, GeneralProtection,    u32 ErrorCode) \
   ELEM(14, PageFault,            u32 ErrorCode) \
   ELEM(16, FloatingPoint,        void) \
   ELEM(17, AlignmentCheck,       u32 ErrorCode) \
   ELEM(18, MachineCheck,         void) \
   ELEM(19, SIMDFloatingPoint,    void) \
   ELEM(20, Virtualization,       void) \
   ELEM(21, ControlProtection,    u32 ErrorCode) \
   ELEM(28, HypervisorInjection,  void) \
   ELEM(29, VMMCommunication,     u32 ErrorCode) \
   ELEM(30, Security,             u32 ErrorCode)

#define ELEM(Num, Name, ...) \
   void Exception_##Name(__VA_ARGS__);
EXCEPTIONS
#undef ELEM

vptr InterruptHandlers[30] = {
   #define ELEM(Num, Name, ...) \
      Exception_##Name,
   EXCEPTIONS
#undef ELEM
};

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
   IDT->Entries[Index].Attributes = 0b10001111;
   IDT->Entries[Index]._Reserved = 0;
}

internal void
IDT_Init(IN idt *IDT)
{
   #define ELEM(Num, Name, ...) \
      IDT_SetEntry(IDT, Num, Exception_##Name);
   EXCEPTIONS
   #undef ELEM
   
   SetIDTR(IDT, sizeof(idt)-1);
}

#undef EXCEPTIONS

#endif