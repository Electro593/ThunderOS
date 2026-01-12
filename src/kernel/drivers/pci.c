/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                           **
**  Author: Aria Seiler                                                      **
**                                                                           **
**  This program is in the public domain. There is no implied warranty,      **
**  so use it at your own risk.                                              **
**                                                                           **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <drivers/pci.h>
#include <kernel/kernel.h>

u32
PCI_Read32(u32 Bus, u32 Device, u32 Function, u32 Offset)
{
	Assert(Bus < 256 && Device < 32 && Function < 8 && Offset < 256);
	u32 Address = (1 << 31)
				| (Bus << 16)
				| (Device << 11)
				| (Function << 8)
				| (Offset & 0xFC);
	PortOut32(0xCF8, Address);

	u32 Data = PortIn32(0xCFC);
	return Data;
}

static u16
PCI_Read16(u32 Bus, u32 Device, u32 Function, u32 Offset)
{
	u32 Data = PCI_Read32(Bus, Device, Function, Offset);
	return (Data >> ((Offset & 2) * 8)) & 0xFFFF;
}

static u08
PCI_Read08(u32 Bus, u32 Device, u32 Function, u32 Offset)
{
	u32 Data = PCI_Read32(Bus, Device, Function, Offset);
	return (Data >> ((Offset & 3) * 8)) & 0xFF;
}

pci
PCI_Init()
{
	pci PCI = { 0 };

	for (u32 B = 0; B < 256; B++) {
		for (u32 D = 0; D < 32; D++) {
			for (u32 F = 0; F < 8; F++) {
				u16 Vendor = PCI_Read16(B, D, F, 0);
				if (Vendor == PCI_Vendor_Invalid) continue;

				u08 HeaderType = PCI_Read08(B, D, F, 13);

				u32 Reg2	  = PCI_Read32(B, D, F, 8);
				u08 Class	  = (Reg2 >> 24) & 0xFF;
				u08 Subclass  = (Reg2 >> 16) & 0xFF;
				u08 Interface = (Reg2 >> 8) & 0xFF;

				if (Class == PCI_Serial
					&& Subclass == PCI_Serial_USB
					&& Interface == PCI_Serial_USB_XHCI)
					PCI.XHCI = (1 << 31) | (B << 16) | (D << 8) | F;

				if (F == 0 && (HeaderType & 0x80) == 0) break;
			}
		}
	}

	return PCI;
}

static status
PCI_EnableMSI(u08 B, u08 D, u08 F)
{
	u16 Status = PCI_Read16(B, D, F, OFFSETOF(pci_header, Status));
	if (!(Status & PCI_Status_CapabilityList)) return ST_NotSupported;

	b08 HasMSI	= FALSE;
	b08 HasMSIx = FALSE;
	u08 MSIPtr	= 0;

	u08 CapPtr =
		PCI_Read08(B, D, F, OFFSETOF(pci_header, Type0.CapabilitiesPointer));
	CapPtr &= 0xFC;

	while (CapPtr != 0) {
		u32 Reg0 = PCI_Read32(B, D, F, CapPtr);
		u08 ID	 = Reg0 & 0xFF;

		if (ID == PCI_Capability_MSI) {
			HasMSI = TRUE;
			MSIPtr = CapPtr;
#if 1
			break;
		}
#else
		} else if (ID == PCI_Capability_MSIx) {
			HasMSIx = TRUE;
			MSIPtr	= CapPtr;
			break;
		}
#endif

		CapPtr = (Reg0 >> 8) & 0xFC;
	}

	u32 Reg0 = PCI_Read32(B, D, F, MSIPtr);
	if (HasMSIx) {
		return ST_Success;
	} else if (HasMSI) {
		u08 RequestedVectors = (Reg0 >> 17) & 0x7;
		b08 Address64Bit	 = (Reg0 >> 23) & 0x1;
		b08 PerVectorMasking = (Reg0 >> 24) & 0x1;

		return ST_Success;
	} else return ST_NotSupported;
}

#if 0
// #define _CONCAT(A, B) A ## B
// #define PCIHEADER_GET(Field, BusNum, DeviceNum) _CONCAT(PCI_Read, SIZEOF(pci_header, Field))(BusNum, DeviceNum, 0, OFFSETOF(pci_header, Field))
#define PCIHEADER_GET(BusNum, DeviceNum, FuncNum, Field)                       \
  ((PCI_Read32(BusNum, DeviceNum, FuncNum, OFFSETOF(pci_header, Field)) >>     \
    (8 * (OFFSETOF(pci_header, Field) & 3))) &                                 \
   ((1 << (8 * SIZEOF(pci_header, Field))) - 1))

void _PCI_CheckBus(u32 Bus);

static void
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

static void
_PCI_CheckDevice(u32 Bus, u32 Device)
{
    u32 VendorID = PCIHEADER_GET(B,D,F, VendorID);
    if(VendorID == 0xFFFF) return;
    _PCI_CheckFunction(Bus, Device, Function);



}

static void
_PCI_CheckBus(u32 Bus)
{
    for(u32 Device = 0; Device < 32; ++Device)
        _PCI_CheckDevice(Bus, Device);
}

static u32
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
