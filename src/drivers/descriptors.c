/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                         **
**  Author: Aria Seiler                                                    **
**                                                                         **
**  This program is in the public domain. There is no implied warranty,    **
**  so use it at your own risk.                                            **
**                                                                         **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifdef INCLUDE_HEADER

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
   u16 IOMap;
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

extern void SetCS(u16 GDTOffset);
extern void SetSegments(u16 CS, u16 SS);
extern void SetGDTR(vptr GDT, u16 Size);
extern void SetIDTR(idt *IDT, u16 Size);
extern void SetTR(u16 GDTOffset);

#define INTERRUPT_NUMS N(0)N(1)N(2)N(3)N(4)N(5)N(6)N(7)N(8)N(9)N(10)N(11)N(12)N(13)N(14)N(15)N(16)N(17)N(18)N(19)N(20)N(21)N(22)N(23)N(24)N(25)N(26)N(27)N(28)N(29)N(30)N(31)N(32)N(33)N(34)N(35)N(36)N(37)N(38)N(39)N(40)N(41)N(42)N(43)N(44)N(45)N(46)N(47)N(48)N(49)N(50)N(51)N(52)N(53)N(54)N(55)N(56)N(57)N(58)N(59)N(60)N(61)N(62)N(63)N(64)N(65)N(66)N(67)N(68)N(69)N(70)N(71)N(72)N(73)N(74)N(75)N(76)N(77)N(78)N(79)N(80)N(81)N(82)N(83)N(84)N(85)N(86)N(87)N(88)N(89)N(90)N(91)N(92)N(93)N(94)N(95)N(96)N(97)N(98)N(99)N(100)N(101)N(102)N(103)N(104)N(105)N(106)N(107)N(108)N(109)N(110)N(111)N(112)N(113)N(114)N(115)N(116)N(117)N(118)N(119)N(120)N(121)N(122)N(123)N(124)N(125)N(126)N(127)N(128)N(129)N(130)N(131)N(132)N(133)N(134)N(135)N(136)N(137)N(138)N(139)N(140)N(141)N(142)N(143)N(144)N(145)N(146)N(147)N(148)N(149)N(150)N(151)N(152)N(153)N(154)N(155)N(156)N(157)N(158)N(159)N(160)N(161)N(162)N(163)N(164)N(165)N(166)N(167)N(168)N(169)N(170)N(171)N(172)N(173)N(174)N(175)N(176)N(177)N(178)N(179)N(180)N(181)N(182)N(183)N(184)N(185)N(186)N(187)N(188)N(189)N(190)N(191)N(192)N(193)N(194)N(195)N(196)N(197)N(198)N(199)N(200)N(201)N(202)N(203)N(204)N(205)N(206)N(207)N(208)N(209)N(210)N(211)N(212)N(213)N(214)N(215)N(216)N(217)N(218)N(219)N(220)N(221)N(222)N(223)N(224)N(225)N(226)N(227)N(228)N(229)N(230)N(231)N(232)N(233)N(234)N(235)N(236)N(237)N(238)N(239)N(240)N(241)N(242)N(243)N(244)N(245)N(246)N(247)N(248)N(249)N(250)N(251)N(252)N(253)N(254)N(255)

#define N(Num) extern void InterruptSwitch##Num(void);
INTERRUPT_NUMS
#undef N

vptr InterruptHandlers[256];

vptr InterruptSwitches[256] = {
   #define N(Num) InterruptSwitch##Num,
   INTERRUPT_NUMS
   #undef N
};

typedef u08 page[0x1000] __attribute__((aligned(0x1000)));
page ISTStacks[7];
page RingStacks[3];

gdt _GDT;
tss _TSS;

#endif



#ifdef INCLUDE_SOURCE

internal void
GDT_Init(void)
{
   gdt *GDT = &_GDT;
   tss *TSS = &_TSS;
   
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
   TSS->RSP0L = ((s64)&RingStacks[0] + 0x1000) & 0xFFFFFFFF;
   TSS->RSP0H = ((s64)&RingStacks[0] + 0x1000) >> 32;
   TSS->RSP1L = ((s64)&RingStacks[1] + 0x1000) & 0xFFFFFFFF;
   TSS->RSP1H = ((s64)&RingStacks[1] + 0x1000) >> 32;
   TSS->RSP2L = ((s64)&RingStacks[2] + 0x1000) & 0xFFFFFFFF;
   TSS->RSP2H = ((s64)&RingStacks[2] + 0x1000) >> 32;
   TSS->_Reserved1 = 0;
   TSS->IST1L = ((s64)&ISTStacks[0] + 0x1000) & 0xFFFFFFFF;
   TSS->IST1H = ((s64)&ISTStacks[0] + 0x1000) >> 32;
   TSS->IST2L = ((s64)&ISTStacks[1] + 0x1000) & 0xFFFFFFFF;
   TSS->IST2H = ((s64)&ISTStacks[1] + 0x1000) >> 32;
   TSS->IST3L = ((s64)&ISTStacks[2] + 0x1000) & 0xFFFFFFFF;
   TSS->IST3H = ((s64)&ISTStacks[2] + 0x1000) >> 32;
   TSS->IST4L = ((s64)&ISTStacks[3] + 0x1000) & 0xFFFFFFFF;
   TSS->IST4H = ((s64)&ISTStacks[3] + 0x1000) >> 32;
   TSS->IST5L = ((s64)&ISTStacks[4] + 0x1000) & 0xFFFFFFFF;
   TSS->IST5H = ((s64)&ISTStacks[4] + 0x1000) >> 32;
   TSS->IST6L = ((s64)&ISTStacks[5] + 0x1000) & 0xFFFFFFFF;
   TSS->IST6H = ((s64)&ISTStacks[5] + 0x1000) >> 32;
   TSS->IST7L = ((s64)&ISTStacks[6] + 0x1000) & 0xFFFFFFFF;
   TSS->IST7H = ((s64)&ISTStacks[6] + 0x1000) >> 32;
   TSS->_Reserved2 = 0;
   TSS->_Reserved3 = 0;
   TSS->IOMapOffset = 0x0068;
   TSS->IOMap = 0xFFFF;
   
   SetGDTR(GDT, sizeof(gdt)-1);
   
   SetSegments(0x08, 0x10);
   SetTR(0x18);
}

internal void
IDT_SetEntry(idt *IDT, u32 Index, vptr Handler)
{
   InterruptHandlers[Index] = Handler;
   
   u64 Address = (u64)InterruptSwitches[Index];
   IDT->Entries[Index].OffsetP1 = (u16)(Address>> 0);
   IDT->Entries[Index].OffsetP2 = (u16)(Address>>16);
   IDT->Entries[Index].OffsetP3 = (u32)(Address>>32);
   IDT->Entries[Index].SegmentSelector = 0x08;
   IDT->Entries[Index].ISTOffset = 0b001;
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