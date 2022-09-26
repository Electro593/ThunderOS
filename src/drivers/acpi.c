/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                         **
**  Author: Aria Seiler                                                    **
**                                                                         **
**  This program is in the public domain. There is no implied warranty,    **
**  so use it at your own risk.                                            **
**                                                                         **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifdef INCLUDE_HEADER

typedef enum acpi_iapc_boot_arch_flags {
    IAPCBootArch_LegacyDevices     = (1<<0),
    IAPCBootArch_8042              = (1<<1),
    IAPCBootArch_VGANotPresent     = (1<<2),
    IAPCBootArch_MSINotSupported   = (1<<3),
    IAPCBootArch_PCIeASPMControls  = (1<<4),
    IAPCBootArch_CMOSRTCNotPresent = (1<<5),
} acpi_iapc_boot_arch_flags;

#pragma pack(push, 1)
typedef struct acpi_rptr {
    u08 AddressSpace;
    u08 BitWidth;
    u08 BitOffset;
    u08 AccessSize;
    u64 Address;
} acpi_rptr;

typedef struct acpi_madt_entry {
    u08 Type;
    u08 Length;
    
    union {
        struct {
            u08 ProcessorID;
            u08 APICID;
            u32 Flags;
        } Type0;
        
        struct {
            u08 IOAPICID;
            u08 _Reserved;
            u32 IOAPICAddress;
            u32 InterruptBase;
        } Type1;
        
        struct {
            u08 BusSource;
            u08 IRQSource;
            u32 GlobalSystemInterrupt;
            u16 Flags;
        } Type2;
        
        struct {
            u08 NMISource;
            u08 _Reserved;
            u16 Flags;
            u32 GlobalSystemInterrupt;
        } Type3;
        
        struct {
            u08 ProcessorID;
            u16 Flags;
            u08 LINTID;
        } Type4;
        
        struct {
            u16 _Reserved;
            u64 LocalAPICAddress;
        } Type5;
        
        struct {
            u16 _Reserved;
            u32 Localx2APICID;
            u32 Flags;
            u32 ACPIID;
        } Type9;
    };
} acpi_madt_entry;

typedef struct acpi_sdt {
    c08 Signature[4];
    u32 Length;
    u08 Revision;
    u08 Checksum;
    c08 OEMID[6];
    c08 OEMTableID[8];
    u32 OEMRevision;
    u32 CreatorID;
    u32 CreatorRevision;
} acpi_sdt;

typedef struct acpi_fadt {
    acpi_sdt Header;
    u32 FirmwareControl;
    u32 DSDT;
    u08 _Reserved0;
    u08 PreferredPowerManagementProfile;
    u16 SCIInterrupt;
    u32 SCICommandPort;
    u08 ACPIEnable;
    u08 ACPIDisable;
    u08 S4BIOSReq;
    u08 PerformanceStateControl;
    u32 PME1aBlockPort;
    u32 PME1bBlockPort;
    u32 PMC1aBlockPort;
    u32 PMC1bBlockPort;
    u32 PMC2BlockPort;
    u32 PMTBlockPort;
    u32 GPE0BlockPort;
    u32 GPE1BlockPort;
    u08 PM1EventLength;
    u08 PM1ControlLength;
    u08 PM2ControlLength;
    u08 PMTimerLength;
    u08 GPE0Length;
    u08 GPE1Length;
    u08 GPE1Base;
    u08 CStateControl;
    u16 WorstC2Latency;
    u16 WorstC3Latency;
    u16 FlushSize;
    u16 FlushStride;
    u08 DutyOffset;
    u08 DutyWidth;
    u08 DayAlarm;
    u08 MonthAlarm;
    u08 Century;
    u16 IAPCBootArch;
    u08 _Reserved1;
    u32 Flag;
    acpi_rptr Reset;
    u08 ResetValue;
    u16 ARMBootArch;
    u08 FADTMinorVersion;
    u64 FirmwareControlX;
    u64 DSDTX;
    acpi_rptr PM1aEBlockX;
    acpi_rptr PM1bEBlockX;
    acpi_rptr PM1aCBlockX;
    acpi_rptr PM1bCBlockX;
    acpi_rptr PM2CBlockX;
    acpi_rptr PMTBlockX;
    acpi_rptr GPE0BlockX;
    acpi_rptr GPE1BlockX;
    acpi_rptr SleepControl;
    acpi_rptr SleepStatus;
    u64 HVIdentity;
} acpi_fadt;

typedef struct acpi_madt {
    acpi_sdt Header;
    u32 LocalAPICAddress;
    u32 Flags;
    acpi_madt_entry Entries[];
} acpi_madt;

typedef struct acpi_xsdt {
    acpi_sdt Header;
    acpi_sdt *Entries[]; // Physical address
} acpi_xsdt;

typedef struct rsdp {
    c08 Signature[8];
    u08 Checksum;
    c08 OEMID[6];
    u08 Revision;
    u32 RSDTAddress;
    u32 Length;
    acpi_xsdt *XSDT;
    u08 ExtendedChecksum;
    u08 _Reserved[3];
} rsdp;
#pragma pack(pop)

typedef struct acpi {
    rsdp *RSDP;
    
    acpi_fadt *FADT;
    acpi_madt *MADT;
    acpi_xsdt *XSDT;
} acpi;

typedef struct local_apic {
    u08 _Reserved0[32];
    u64 LocalAPICIDRegister[2];
    u64 LocalAPICVersionRegister[2];
} local_apic;

#endif



#ifdef INCLUDE_SOURCE

extern vptr APICBase;
vptr APICBase;

internal b08
ValidateRSDP(rsdp *RSDP)
{
    if(RSDP->Revision != 2)
        return FALSE;
    
    u08 Sum = 0;
    for(u32 ByteIndex = 0; ByteIndex < sizeof(rsdp); ++ByteIndex)
        Sum += ((u08*)RSDP)[ByteIndex];
    if(Sum != 0)
        return FALSE;
    
    return TRUE;
}

internal b08
ValidateSDT(vptr SDT)
{
    u32 Size = ((acpi_sdt*)SDT)->Length;
    
    u08 Sum = 0;
    for(u32 ByteIndex = 0; ByteIndex < Size; ++ByteIndex)
        Sum += ((u08*)SDT)[ByteIndex];
    if(Sum != 0)
        return FALSE;
    
    return TRUE;
}

internal acpi
InitACPI(rsdp *RSDP)
{
    acpi ACPI = {0};
    
    ACPI.RSDP = RSDP;
    Assert(ValidateRSDP(ACPI.RSDP));
    
    ACPI.XSDT = RSDP->XSDT;
    Assert(ValidateSDT(ACPI.XSDT));
    
    u32 SDTCount = (ACPI.XSDT->Header.Length - sizeof(acpi_sdt)) / sizeof(vptr);
    for(u32 SDTIndex = 0; SDTIndex < SDTCount; ++SDTIndex) {
        acpi_sdt *SDT = ACPI.XSDT->Entries[SDTIndex];
        
        if(Mem_Cmp(SDT, "FACP", SIZEOF(acpi_sdt, Signature)) == EQUAL) {
            ACPI.FADT = (acpi_fadt*)SDT;
            Assert(ValidateSDT(ACPI.FADT));
        } else if(Mem_Cmp(SDT, "APIC", SIZEOF(acpi_sdt, Signature)) == EQUAL) {
            ACPI.MADT = (acpi_madt*)SDT;
            Assert(ValidateSDT(ACPI.MADT));
        }
    }
    
    return ACPI;
}

internal thunderos_status
InitAPIC(acpi ACPI)
{
    // Disable PIC
    PortOut08(0x20, 0x11);
    PortOut08(0xA0, 0x11);
    
    PortOut08(0x21, 0xE0);
    PortOut08(0xA1, 0xE8);
    
    PortOut08(0x21, 0x04);
    PortOut08(0xA1, 0x02);
    
    PortOut08(0x21, 0x01);
    PortOut08(0xA1, 0x01);
    
    PortOut08(0x21, 0xFF);
    PortOut08(0xA1, 0xFF);
    
    acpi_madt *MADT = ACPI.MADT;
    u08 *LocalAPIC = (u08*)(u64)MADT->LocalAPICAddress;
    // u08 *IOAPIC = NULL;
    
    u32 Offset = 0;
    while(Offset < MADT->Header.Length - sizeof(acpi_sdt)) {
        acpi_madt_entry *Entry = (acpi_madt_entry*)((u08*)MADT->Entries + Offset);
        
        switch(Entry->Type) {
            // case 1: {
            //     if(Entry->Type1.InterruptBase == 0) {
            //         IOAPIC = (u08*)Entry->Type1.IOAPICAddress;
            //     }
            // } break;
            case 5: {
                LocalAPIC = (u08*)Entry->Type5.LocalAPICAddress;
            } break;
        }
        
        Offset += Entry->Length;
    }
    
    APICBase = LocalAPIC;
    //SetMSR(0x1B, GetMSR(0x1B)|0x800);
    
    u32 *SpuriousInterruptVector = (u32*)(LocalAPIC + 0x0F0);
    *SpuriousInterruptVector |= 0x00000100;
    
    return ST_Success;
}

#endif