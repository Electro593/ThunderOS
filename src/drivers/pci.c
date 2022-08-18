/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                         **
**  Author: Aria Seiler                                                    **
**                                                                         **
**  This program is in the public domain. There is no implied warranty,    **
**  so use it at your own risk.                                            **
**                                                                         **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifdef INCLUDE_HEADER

typedef enum pci_vendor_id {
    PCI_Vendor_Invalid = 0xFFFF
} pci_vendor_id;

typedef enum pci_class_ids {
    PCI_Unclassified                                     = 0x00,
    PCI_Unclassified_NonVGACompatible                    = 0x00,
    PCI_Unclassified_VGACompatible                       = 0x01,
    PCI_MassStorage                                      = 0x01,
    PCI_MassStorage_SCSIBus                              = 0x00,
    PCI_MassStorage_IDE                                  = 0x01,
    PCI_MassStorage_IDE_ISA                              = 0x00,
    PCI_MassStorage_IDE_PCI                              = 0x05,
    PCI_MassStorage_IDE_ISAWithPCISupport                = 0x0A,
    PCI_MassStorage_IDE_PCIWithISASupport                = 0x0F,
    PCI_MassStorage_IDE_ISAWithBusMastering              = 0x80,
    PCI_MassStorage_IDE_PCIWithBusMastering              = 0x85,
    PCI_MassStorage_IDE_ISAWithPCISupportAndBusMastering = 0x8A,
    PCI_MassStorage_IDE_PCIWithISASupportAndBusMastering = 0x8F,
    PCI_MassStorage_FloppyDisk                           = 0x02,
    PCI_MassStorage_IPIBus                               = 0x03,
    PCI_MassStorage_RAID                                 = 0x04,
    PCI_MassStorage_ATA                                  = 0x05,
    PCI_MassStorage_ATA_Single                           = 0x00,
    PCI_MassStorage_ATA_Chained                          = 0x01,
    PCI_MassStorage_SerialATA                            = 0x06,
    PCI_MassStorage_SerialATA_VendorSpecific             = 0x00,
    PCI_MassStorage_SerialATA_AHCI                       = 0x01,
    PCI_MassStorage_SerialATA_StorageBus                 = 0x02,
    PCI_MassStorage_SerialAttachedSCSI                   = 0x07,
    PCI_MassStorage_SerialAttachedSCSI_SAS               = 0x00,
    PCI_MassStorage_SerialAttachedSCSI_StorageBus        = 0x01,
    PCI_MassStorage_NonVolatileMemory                    = 0x08,
    PCI_MassStorage_NonVolatileMemory_NVMHCI             = 0x01,
    PCI_MassStorage_NonVolatileMemory_NVMExpress         = 0x02,
    PCI_MassStorage_Other                                = 0x80,
    PCI_Network                                          = 0x02,
    PCI_Network_Ethernet                                 = 0x00,
    PCI_Network_TokenRing                                = 0x01,
    PCI_Network_FDDI                                     = 0x02,
    PCI_Network_ATM                                      = 0x03,
    PCI_Network_ISDN                                     = 0x04,
    PCI_Network_WorldFip                                 = 0x05,
    PCI_Network_PICMGMultiComputing                      = 0x06,
    PCI_Network_InfiniBand                               = 0x07,
    PCI_Network_Fabric                                   = 0x08,
    PCI_Network_Other                                    = 0x80,
    PCI_Display                                          = 0x03,
    PCI_Display_VGA                                      = 0x00,
    PCI_Display_VGA_Generic                              = 0x00,
    PCI_Display_VGA_8514                                 = 0x01,
    PCI_Display_XGA                                      = 0x01,
    PCI_Display_NonVGACompatible3D                       = 0x02,
    PCI_Display_Other                                    = 0x80,
    PCI_Multimedia                                       = 0x04,
    PCI_Multimedia_VideoController                       = 0x00,
    PCI_Multimedia_AudioController                       = 0x01,
    PCI_Multimedia_ComputerTelephonyDevice               = 0x02,
    PCI_Multimedia_AudioDevice                           = 0x03,
    PCI_Multimedia_Other                                 = 0x80,
    PCI_Memory                                           = 0x05,
    PCI_Memory_RAM                                       = 0x00,
    PCI_Memory_Flash                                     = 0x01,
    PCI_Memory_Other                                     = 0x80,
    PCI_Bridge                                           = 0x06,
    PCI_Bridge_Host                                      = 0x00,
    PCI_Bridge_ISA                                       = 0x01,
    PCI_Bridge_EISA                                      = 0x02,
    PCI_Bridge_MCA                                       = 0x03,
    PCI_Bridge_PCIToPCI                                  = 0x04,
    PCI_Bridge_PCIToPCI_Normal                           = 0x00,
    PCI_Bridge_PCIToPCI_Subtractive                      = 0x01,
    PCI_Bridge_PCMCIA                                    = 0x05,
    PCI_Bridge_NuBus                                     = 0x06,
    PCI_Bridge_CardBus                                   = 0x07,
    PCI_Bridge_RACEway                                   = 0x08,
    PCI_Bridge_RACEway_Transparent                       = 0x00,
    PCI_Bridge_RACEway_Endpoint                          = 0x01,
    PCI_Bridge_SemiTransparentPCIToPCI                   = 0x09,
    PCI_Bridge_SemiTransparentPCIToPCI_Primary           = 0x40,
    PCI_Bridge_SemiTransparentPCIToPCI_Secondary         = 0x80,
    PCI_Bridge_InfiniBandToPCIHost                       = 0x0A,
    PCI_Bridge_Other                                     = 0x80,
    PCI_Communication                                    = 0x07,
    PCI_Communication_Serial                             = 0x00,
    PCI_Communication_Serial_8250                        = 0x00,
    PCI_Communication_Serial_16450                       = 0x01,
    PCI_Communication_Serial_16550                       = 0x02,
    PCI_Communication_Serial_16650                       = 0x03,
    PCI_Communication_Serial_16750                       = 0x04,
    PCI_Communication_Serial_16850                       = 0x05,
    PCI_Communication_Serial_16950                       = 0x06,
    PCI_Communication_Parallel                           = 0x01,
    PCI_Communication_Parallel_Standard                  = 0x00,
    PCI_Communication_Parallel_BiDirectional             = 0x01,
    PCI_Communication_Parallel_ECP                       = 0x02,
    PCI_Communication_Parallel_IEEE                      = 0x03,
    PCI_Communication_Parallel_IEEETarget                = 0xFE,
    PCI_Communication_MultiportSerial                    = 0x02,
    PCI_Communication_Modem                              = 0x03,
    PCI_Communication_Modem_Generic                      = 0x00,
    PCI_Communication_Modem_Hayes16450                   = 0x01,
    PCI_Communication_Modem_Hayes16550                   = 0x02,
    PCI_Communication_Modem_Hayes16650                   = 0x03,
    PCI_Communication_Modem_Hayes16750                   = 0x04,
    PCI_Communication_GPIB                               = 0x04,
    PCI_Communication_SmartCard                          = 0x05,
    PCI_Communication_Other                              = 0x80,
    PCI_Peripheral                                       = 0x08,
    PCI_Peripheral_PIC                                   = 0x00,
    PCI_Peripheral_PIC_Generic                           = 0x00,
    PCI_Peripheral_PIC_ISACompatible                     = 0x01,
    PCI_Peripheral_PIC_EISACompatible                    = 0x02,
    PCI_Peripheral_PIC_APIC                              = 0x10,
    PCI_Peripheral_PIC_APICx                             = 0x20,
    PCI_Peripheral_DMA                                   = 0x01,
    PCI_Peripheral_DMA_Generic                           = 0x00,
    PCI_Peripheral_DMA_ISACompatible                     = 0x01,
    PCI_Peripheral_DMA_EISACompatible                    = 0x02,
    PCI_Peripheral_Timer                                 = 0x02,
    PCI_Peripheral_Timer_Generic                         = 0x00,
    PCI_Peripheral_Timer_ISACompatible                   = 0x01,
    PCI_Peripheral_Timer_EISACompatible                  = 0x02,
    PCI_Peripheral_Timer_HPET                            = 0x03,
    PCI_Peripheral_RTC                                   = 0x03,
    PCI_Peripheral_RTC_Generic                           = 0x00,
    PCI_Peripheral_RTC_ISACompatible                     = 0x01,
    PCI_Peripheral_PCIHotPlug                            = 0x04,
    PCI_Peripheral_SDHost                                = 0x05,
    PCI_Peripheral_IOMMU                                 = 0x06,
    PCI_Peripheral_Other                                 = 0x80,
    PCI_Input                                            = 0x09,
    PCI_Input_Keyboard                                   = 0x00,
    PCI_Input_DigitizerPen                               = 0x01,
    PCI_Input_Mouse                                      = 0x02,
    PCI_Input_Scanner                                    = 0x03,
    PCI_Input_Gameport                                   = 0x04,
    PCI_Input_Gameport_Generic                           = 0x00,
    PCI_Input_Gameport_Extended                          = 0x01,
    PCI_Input_Other                                      = 0x80,
    PCI_Docking                                          = 0x0A,
    PCI_Docking_Generic                                  = 0x00,
    PCI_Docking_Other                                    = 0x80,
    PCI_Processor                                        = 0x0B,
    PCI_Processor_386                                    = 0x00,
    PCI_Processor_486                                    = 0x01,
    PCI_Processor_Pentium                                = 0x02,
    PCI_Processor_PentiumPro                             = 0x03,
    PCI_Processor_Alpha                                  = 0x10,
    PCI_Processor_PowerPC                                = 0x20,
    PCI_Processor_MIPS                                   = 0x30,
    PCI_Processor_CoProcessor                            = 0x40,
    PCI_Processor_Other                                  = 0x80,
    PCI_Serial                                           = 0x0C,
    PCI_Serial_FireWire                                  = 0x00,
    PCI_Serial_FireWire_Generic                          = 0x00,
    PCI_Serial_FireWire_OHCI                             = 0x10,
    PCI_Serial_ACCESSBus                                 = 0x01,
    PCI_Serial_SSA                                       = 0x02,
    PCI_Serial_USB                                       = 0x03,
    PCI_Serial_USB_UHCI                                  = 0x00,
    PCI_Serial_USB_OHCI                                  = 0x10,
    PCI_Serial_USB_EHCI                                  = 0x20,
    PCI_Serial_USB_XHCI                                  = 0x30,
    PCI_Serial_USB_Unspecified                           = 0x80,
    PCI_Serial_USB_Device                                = 0xFE,
    PCI_Serial_Fibre                                     = 0x04,
    PCI_Serial_SMBus                                     = 0x05,
    PCI_Serial_InfiniBand                                = 0x06,
    PCI_Serial_IPMI                                      = 0x07,
    PCI_Serial_IPMI_SMIC                                 = 0x00,
    PCI_Serial_IPMI_Keyboard                             = 0x01,
    PCI_Serial_IPMI_BlockTransfer                        = 0x02,
    PCI_Serial_SERCOSInterface                           = 0x08,
    PCI_Serial_CANbus                                    = 0x09,
    PCI_Serial_Other                                     = 0x80,
    PCI_Wireless                                         = 0x0D,
    PCI_Wireless_iRDACompatible                          = 0x00,
    PCI_Wireless_ConsumerIR                              = 0x01,
    PCI_Wireless_RF                                      = 0x10,
    PCI_Wireless_Bluetooth                               = 0x11,
    PCI_Wireless_Broadband                               = 0x12,
    PCI_Wireless_EthernetA                               = 0x20,
    PCI_Wireless_EthernetB                               = 0x21,
    PCI_Wireless_Other                                   = 0x80,
    PCI_Intelligent                                      = 0x0E,
    PCI_Intelligent_120                                  = 0x00,
    PCI_Satellite                                        = 0x0F,
    PCI_Satellite_TV                                     = 0x01,
    PCI_Satellite_Audio                                  = 0x02,
    PCI_Satellite_Voice                                  = 0x03,
    PCI_Satellite_Data                                   = 0x04,
    PCI_Encryption                                       = 0x10,
    PCI_Encryption_NetworkAndComputing                   = 0x00,
    PCI_Encryption_Entertainment                         = 0x10,
    PCI_Encryption_Other                                 = 0x80,
    PCI_Signal                                           = 0x11,
    PCI_Signal_DPIOModules                               = 0x00,
    PCI_Signal_PerformanceCounters                       = 0x01,
    PCI_Signal_CommunicationSynchronizer                 = 0x10,
    PCI_Signal_ProcessingManagement                      = 0x20,
    PCI_Signal_Other                                     = 0x80,
    PCI_Accelerator                                      = 0x12,
    PCI_NonEssential                                     = 0x13,
    PCI_CoProcessor                                      = 0x40,
    PCI_Unassigned                                       = 0xFF
} pci_class_ids;

typedef enum pci_status_bits {
    PCI_Status_InterruptStatus       = 0x0008,
    PCI_Status_CapabilityList        = 0x0010,
    PCI_Status_66MHzCapable          = 0x0020,
    PCI_Status_FastBackToBackCapable = 0x0080,
    PCI_Status_MasterDataParityError = 0x0100,
    PCI_Status_DEVSELTiming          = 0x0600,
    PCI_Status_SignaledTargetAbort   = 0x0800,
    PCI_Status_ReceivedTargetAbort   = 0x1000,
    PCI_Status_ReceivedMasterAbort   = 0x2000,
    PCI_Status_SignaledSystemError   = 0x4000,
    PCI_Status_DetectedParityError   = 0x8000,
} pci_status_bits;

typedef enum pci_capability_ids {
    PCI_Capability_Null   = 0x00,
    PCI_Capability_PMI    = 0x01,
    PCI_Capability_AGP    = 0x02,
    PCI_Capability_VPD    = 0x03,
    PCI_Capability_SlotID = 0x04,
    PCI_Capability_MSI    = 0x05,
    PCI_Capability_PHS    = 0x06,
    PCI_Capability_PCIx   = 0x07,
    PCI_Capability_HT     = 0x08,
    PCI_Capability_VS     = 0x09,
    PCI_Capability_DP     = 0x0A,
    PCI_Capability_CCRC   = 0x0B,
    PCI_Capability_PHP    = 0x0C,
    PCI_Capability_PBSVID = 0x0D,
    PCI_Capability_AGP8x  = 0x0E,
    PCI_Capability_SD     = 0x0F,
    PCI_Capability_PCIe   = 0x10,
    PCI_Capability_MSIx   = 0x11,
    PCI_Capability_ATA    = 0x12,
    PCI_Capability_AF     = 0x13,
    PCI_Capability_EA     = 0x14,
    PCI_Capability_FPB    = 0x15,
} pci_capability_ids;

typedef struct pci_header {
    u16 VendorID;
    u16 DeviceID;
    u16 Command;
    u16 Status;
    u08 RevisionID;
    u08 Interface;
    u08 Subclass;
    u08 Class;
    u08 CacheLineSize;
    u08 LatencyTimer;
    u08 HeaderType;
    u08 BIST;
    
    union {
        struct pci_header_type_0 {
            u32 BaseAddress0;
            u32 BaseAddress1;
            u32 BaseAddress2;
            u32 BaseAddress3;
            u32 BaseAddress4;
            u32 BaseAddress5;
            u32 CardbusCISPointer;
            u16 SubsystemVendorID;
            u16 SubsystemID;
            u32 ExpansionROMBaseAddress;
            u08 CapabilitiesPointer;
            u08 _Reserved[7];
            u08 InterruptLine;
            u08 InterruptPin;
            u08 MinGrant;
            u08 MaxLatency;
        } Type0;
        
        struct pci_header_type_1 {
            u32 BaseAddress0;
            u32 BaseAddress1;
            u08 PrimaryBusNumber;
            u08 SecondaryBusNumber;
            u08 SubordinateBusNumber;
            u08 SecondaryLatencyTimer;
            u08 IOBase;
            u08 IOLimit;
            u16 SecondaryStatus;
            u16 MemoryBase;
            u16 MemoryLimit;
            u16 PrefetchableMemoryBase;
            u16 PrefetchableMemoryLimit;
            u32 PrefetchableBaseUpper;
            u32 PrefetchableLimitUpper;
            u16 IOBaseUpper;
            u16 IOLimitUpper;
            u08 CapabilityPointer;
            u08 _Reserved[3];
            u32 ExpansionROMBaseAddress;
            u08 InterruptLine;
            u08 InterruptPin;
            u16 BridgeControl;
        } Type1;
        
        struct pci_header_type_2 {
            u32 CardBusSocket;
            u08 CapabilitiesListOffset;
            u08 _Reserved[1];
            u16 SecondaryStatus;
            u08 PCIBusNumber;
            u08 CardBusBusNumber;
            u08 SuboordinateBusNumber;
            u08 CardBusLatencyTimer;
            u32 MemoryBaseAddress0;
            u32 MemoryLimit0;
            u32 MemoryBaseAddress1;
            u32 MemoryLimit1;
            u32 IOBaseAddress0;
            u32 IOLimit0;
            u32 IOBaseAddress1;
            u32 IOLimit1;
            u08 InterruptLine;
            u08 InterruptPin;
            u16 BridgeControl;
            u16 SubsystemDeviceID;
            u16 SubsystemVendorID;
            u32 LegacyModeBaseAddress;
        } Type2;
    };
} pci_header;

typedef struct pci {
    u32 XHCI;
} pci;

#endif



#ifdef INCLUDE_SOURCE

internal u32
PCI_Read32(u32 Bus, u32 Device, u32 Function, u32 Offset)
{
    Assert(Bus < 256 && Device < 32 && Function < 8 && Offset < 256);
    u32 Address = (1<<31)|(Bus<<16)|(Device<<11)|(Function<<8)|(Offset&0xFC);
    PortOut32(0xCF8, Address);
    
    u32 Data = PortIn32(0xCFC);
    return Data;
}

internal u16
PCI_Read16(u32 Bus, u32 Device, u32 Function, u32 Offset)
{
    u32 Data = PCI_Read32(Bus, Device, Function, Offset);
    return (Data >> ((Offset&2)*8)) & 0xFFFF;
}

internal u08
PCI_Read08(u32 Bus, u32 Device, u32 Function, u32 Offset)
{
    u32 Data = PCI_Read32(Bus, Device, Function, Offset);
    return (Data >> ((Offset&3)*8)) & 0xFF;
}

internal pci
PCI_Init()
{
    pci PCI = {0};
    
    for(u32 B = 0; B < 256; B++) {
        for(u32 D = 0; D < 32; D++) {
            for(u32 F = 0; F < 8; F++) {
                u16 Vendor = PCI_Read16(B, D, F, 0);
                if(Vendor == PCI_Vendor_Invalid) continue;
                
                u08 HeaderType = PCI_Read08(B, D, F, 13);
                
                u32 Reg2 = PCI_Read32(B, D, F, 8);
                u08 Class = (Reg2 >> 24) & 0xFF;
                u08 Subclass = (Reg2 >> 16) & 0xFF;
                u08 Interface = (Reg2 >> 8) & 0xFF;
                
                if(Class == PCI_Serial && Subclass == PCI_Serial_USB && Interface == PCI_Serial_USB_XHCI)
                    PCI.XHCI = (1<<31) | (B<<16) | (D<<8) | F;
                
                if(F == 0 && (HeaderType & 0x80) == 0) break;
            }
        }
    }
    
    return PCI;
}

internal thunderos_status
PCI_EnableMSI(u08 B, u08 D, u08 F)
{
    u16 Status = PCI_Read16(B, D, F, OFFSETOF(pci_header, Status));
    if(!(Status & PCI_Status_CapabilityList)) return ST_NotSupported;
    
    b08 HasMSI = FALSE;
    b08 HasMSIx = FALSE;
    u08 MSIPtr = 0;
    
    u08 CapPtr = PCI_Read08(B, D, F, OFFSETOF(pci_header, Type0.CapabilitiesPointer));
    CapPtr &= 0xFC;
    
    while(CapPtr != 0) {
        u32 Reg0 = PCI_Read32(B, D, F, CapPtr);
        u08 ID = Reg0 & 0xFF;
        
        if(ID == PCI_Capability_MSI) {
            HasMSI = TRUE;
            MSIPtr = CapPtr;
        #if 1
            break;
        }
        #else
        } else if(ID == PCI_Capability_MSIx) {
            HasMSIx = TRUE;
            MSIPtr = CapPtr;
            break;
        }
        #endif
        
        CapPtr = (Reg0 >> 8) & 0xFC;
    }
    
    u32 Reg0 = PCI_Read32(B, D, F, MSIPtr);
    if(HasMSIx) {
        
        
        return ST_Success;
    } else if(HasMSI) {
        u08 RequestedVectors = (Reg0 >> 17) & 0x7;
        b08 Address64Bit     = (Reg0 >> 23) & 0x1;
        b08 PerVectorMasking = (Reg0 >> 24) & 0x1;
        
        return ST_Success;
    } else return ST_NotSupported;
}


#if 0
// #define _CONCAT(A, B) A ## B
// #define PCIHEADER_GET(Field, BusNum, DeviceNum) _CONCAT(PCI_Read, SIZEOF(pci_header, Field))(BusNum, DeviceNum, 0, OFFSETOF(pci_header, Field))
#define PCIHEADER_GET(BusNum, DeviceNum, FuncNum, Field) ((PCI_Read32(BusNum, DeviceNum, FuncNum, OFFSETOF(pci_header, Field)) >> (8*(OFFSETOF(pci_header, Field)&3))) & ((1<<(8*SIZEOF(pci_header, Field))) - 1))

void _PCI_CheckBus(u32 Bus);

internal void
_PCI_CheckFunction(u32 Bus, u32 Device, u32 Function)
{
    u32 Class = PCIHEADER_GET(B,D,F, Class);
    u32 Subclass = PCIHEADER_GET(B,D,F, Subclass);
    if(Class    == PCI_Class_Bridge &&
       Subclass == PCI_Subclass_PCIToPCIBridge)
    {
        u32 SecondaryBus = PCIHEADER_GET(B,D,F, Type1.SecondaryBusNumber);
        _PCI_CheckBus(SecondaryBus);
    }
}

internal void
_PCI_CheckDevice(u32 Bus, u32 Device)
{
    u32 VendorID = PCIHEADER_GET(B,D,F, VendorID);
    if(VendorID == 0xFFFF) return;
    _PCI_CheckFunction(Bus, Device, Function);
    
    
    
}

internal void
_PCI_CheckBus(u32 Bus)
{
    for(u32 Device = 0; Device < 32; ++Device)
        _PCI_CheckDevice(Bus, Device);
}

internal u32
PCI_InitDeviceTable(OUT vptr Table)
{
    // for(u32 B = 0; B < 256; ++B) // B = BusIndex
    // {
    //     for(u32 D = 0; D < 32; ++D) // D = DeviceIndex
    //     {
    //         u32 F = 0; // F = Function
            
    //     }
    // }
    
    u32 HeaderType = PCIHEADER_GET(0,0,0, HeaderType);
    if((HeaderType & 0x80) == 0) {
        _PCI_CheckBus(0);
    } else {
        for(u32 Function = 0; Function < 8; ++Function)
        {
            if(PCIHEADER_GET(0,0,Function, VendorID) != 0xFFFF) break;
        }
    }
}
#endif

#endif