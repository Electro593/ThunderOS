/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                         **
**  Author: Aria Seiler                                                    **
**                                                                         **
**  This program is in the public domain. There is no implied warranty,    **
**  so use it at your own risk.                                            **
**                                                                         **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */












































#if 0
typedef enum pci_vendor_id {
    PCI_Vendor_Invalid = 0xFFFF
} pci_vendor_id;

typedef enum pci_class_id {
    PCI_Class_Unclassified                     = 0x00,
    PCI_Class_MassStorageController            = 0x01,
    PCI_Class_NetworkController                = 0x02,
    PCI_Class_DisplayController                = 0x03,
    PCI_Class_MultimediaController             = 0x04,
    PCI_Class_MemoryController                 = 0x05,
    PCI_Class_Bridge                           = 0x06,
    PCI_Class_SimpleCommunicationController    = 0x07,
    PCI_Class_BaseSystemPeripheral             = 0x08,
    PCI_Class_InputDeviceController            = 0x09,
    PCI_Class_DockingStation                   = 0x0A,
    PCI_Class_Processor                        = 0x0B,
    PCI_Class_SerialBusController              = 0x0C,
    PCI_Class_WirelessController               = 0x0D,
    PCI_Class_IntelligentController            = 0x0E,
    PCI_Class_SatelliteCommunicationController = 0x0F,
    PCI_Class_EncryptionController             = 0x10,
    PCI_Class_SignalProcessingController       = 0x11,
    PCI_Class_ProcessingAccelerator            = 0x12,
    PCI_Class_NonEssentialInstrumentation      = 0x13,
    PCI_Class_CoProcessor                      = 0x40,
    PCI_Class_Unassigned                       = 0xFF
} pci_class_id;

typedef enum pci_subclass_id {
    // Class 0x06
    PCI_Subclass_PCIToPCIBridge = 0x04,
    
    // Class 0x0C
    PCI_Subclass_USBController = 0x03,
} pci_subclass_id;

typedef enum pci_interface_id {
    PCI_Interface_XHCIController = 0x30,
} pci_interface_id;

typedef struct pci_header {
    u16 DeviceID;
    u16 VendorID;
    u16 Status;
    u16 Command;
    u08 Class;
    u08 Subclass;
    u08 Interface;
    u08 RevisionID;
    u08 BIST;
    u08 HeaderType;
    u08 LatencyTimer;
    u08 CacheLineSize;
    
    union {
        struct pci_header_type_0 {
            u32 BaseAddress0;
            u32 BaseAddress1;
            u32 BaseAddress2;
            u32 BaseAddress3;
            u32 BaseAddress4;
            u32 BaseAddress5;
            u32 CardbusCISPointer;
            u16 SubsystemID;
            u16 SubsystemVendorID;
            u32 ExpansionROMBaseAddress;
            u08 _Reserved0[3];
            u08 CapabilitiesPointer;
            u08 _Reserved1[4];
            u08 MaxLatency;
            u08 MinGrant;
            u08 InterruptPin;
            u08 InterruptLine;
        } Type0;
        
        struct pci_header_type_1 {
            u32 BaseAddress0;
            u32 BaseAddress1;
            u08 SecondaryLatencyTimer;
            u08 SubordinateBusNumber;
            u08 SecondaryBusNumber;
            u08 PrimaryBusNumber;
            u16 SecondaryStatus;
            u08 IOLimit;
            u08 IOBase;
            u16 MemoryLimit;
            u16 MemoryBase;
            u16 PrefetchableMemoryLimit;
            u16 PrefetchableMemoryBase;
            u32 PrefetchableBaseUpper;
            u32 PrefetchableLimitUpper;
            u16 IOLimitUpper;
            u16 IOBaseUpper;
            u08 _Reserved[3];
            u08 CapabilityPointer;
            u32 ExpansionROMBaseAddress;
            u16 BridgeControl;
            u08 InterruptPin;
            u08 InterruptLine;
        } Type1;
        
        struct pci_header_type_2 {
            u32 CardBusSocket;
            u16 SecondaryStatus;
            u08 _Reserved[1];
            u08 CapabilitiesListOffset;
            u08 CardBusLatencyTimer;
            u08 SuboordinateBusNumber;
            u08 CardBusBusNumber;
            u08 PCIBusNumber;
            u32 MemoryBaseAddress0;
            u32 MemoryLimit0;
            u32 MemoryBaseAddress1;
            u32 MemoryLimit1;
            u32 IOBaseAddress0;
            u32 IOLimit0;
            u32 IOBaseAddress1;
            u32 IOLimit1;
            u16 BridgeControl;
            u08 InterruptPin;
            u08 InterruptLine;
            u16 SubsystemVendorID;
            u16 SubsystemDeviceID;
            u32 LegacyModeBaseAddress;
        } Type2;
    };
} pci_header;

// #define _CONCAT(A, B) A ## B
// #define PCIHEADER_GET(Field, BusNum, DeviceNum) _CONCAT(PCI_Read, SIZEOF(pci_header, Field))(BusNum, DeviceNum, 0, OFFSETOF(pci_header, Field))
#define PCIHEADER_GET(BusNum, DeviceNum, FuncNum, Field) ((PCI_Read32(BusNum, DeviceNum, FuncNum, OFFSETOF(pci_header, Field)) >> (8*(OFFSETOF(pci_header, Field)&3))) & ((1<<(8*SIZEOF(pci_header, Field))) - 1))

internal u32
PCI_Read32(u32 BusNum,
           u32 DeviceNum,
           u32 FuncNum,
           u32 Offset)
{
    u32 Enabled = 1 << 31;
    BusNum <<= 16;
    DeviceNum <<= 11;
    FuncNum <<= 8;
    Offset &= ~3;
    
    u32 Address = Enabled|BusNum|DeviceNum|FuncNum|Offset;
    PortOut32(0xCF8, Address);
    
    u32 Data = PortIn32(0xCFC);
    return Data;
}

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