/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                           **
**  Author: Aria Seiler                                                      **
**                                                                           **
**  This program is in the public domain. There is no implied warranty,      **
**  so use it at your own risk.                                              **
**                                                                           **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <util/intrin.h>
#include <util/mem.h>
#include <util/str.h>
#include <util/vector.h>
#include <drivers/acpi.h>
#include <drivers/descriptors.h>
#include <drivers/mem.h>
#include <drivers/pci.h>
#include <drivers/serial.h>
#include "kernel.h"

#include <loader/efi.h>

#define MAKE_DUMMY(Name)                                                       \
  void Name() { KernelError(__FILE__, __LINE__, 0, #Name " does not exist"); }
MAKE_DUMMY(sigemptyset);
MAKE_DUMMY(sigaction);
MAKE_DUMMY(backtrace);
MAKE_DUMMY(backtrace_symbols);
MAKE_DUMMY(_exit);
MAKE_DUMMY(write);
MAKE_DUMMY(memcpy);
MAKE_DUMMY(memset);
MAKE_DUMMY(memcmp);
MAKE_DUMMY(stbsp_sprintf);
MAKE_DUMMY(pthread_mutex_init);
MAKE_DUMMY(pthread_mutexattr_init);
MAKE_DUMMY(pthread_mutexattr_settype);
MAKE_DUMMY(pthread_mutex_lock);
MAKE_DUMMY(pthread_mutex_unlock);

global_state Global;

typedef struct __attribute__((packed)) bitmap_header {
	c08 Signature[2];
	u32 FileSize;
	u32 Reserved;
	u32 DataOffset;
	u32 Size;
	u32 Width;
	u32 Height;
	u16 Planes;
	u16 BitsPerPixel;
	u32 Compression;
	u32 ImageSize;
	u32 XPixelsPerM;
	u32 YPixelsPerM;
	u32 ColorsUsed;
	u32 ImportantColors;
} bitmap_header;

// TODO: Remove dependencies to everything
void
KernelError(c08 *File, u32 Line, c08 *Expression, c08 *Message)
{
	if (Global.Flags & HW_HasSerial) {
		c08 Buffer[64];

		Serial_Write(Global.SerialPort, "\r\nERROR in ");
		Serial_Write(Global.SerialPort, File);
		Serial_Write(Global.SerialPort, " on line ");
		Serial_Write(Global.SerialPort, U64_ToStr(Buffer, Line, 10));
		Serial_Write(Global.SerialPort, ": ");
		Serial_Write(Global.SerialPort, Message);
		Serial_Write(Global.SerialPort, ".\r\n\n\t");
		Serial_Write(Global.SerialPort, Expression);
		Serial_Write(Global.SerialPort, "\r\n");
	}

	__asm__("hlt");
}

external u32
Kernel_Entry(
	rsdp							*RSDP,
	efi_graphics_output_protocol	*GOP,
	efi_pci_root_bridge_io_protocol *PRBIP,
	efi_simple_file_system_protocol *SFSP,
	efi_memory_descriptor			*MemoryMap,
	u64								 MemoryMapDescriptorSize,
	u32								 MemoryMapDescriptorCount
)
{
	u32 Status;

	DisableInterrupts();

	// Set up the page table
	{
		SetCR0(GetCR0() & ~CR0_WriteProtect);

		b08	 HasLvl5  = !!(GetCR4() & CR4_57BitLinearAddress);
		u64	 PageAddr = (s64) (GetCR3() & 0x000FFFFFFFFFF000) << 12 >> 12;
		u64 *Entries  = (u64 *) PageAddr;

		// Recursively map CR3
		Entries[511] = PageAddr | VMap_Present | VMap_WriteAccess;
		InvalidatePage((vptr) PageAddr);

		// HACK: Write-enable the table representing the page map
		*(u64 *) 0xFFFFFFFFC0000FF0 |= VMap_WriteAccess;
		InvalidatePage((vptr) 0xFFFFFFFFC0000FF0);

		SetCR0(GetCR0() | CR0_WriteProtect);
	}

	PMap = &_PMap;
	Mem_Set(PMap, 0xFF, sizeof(pmap_leaf));

	// Initialize as if all pages are being used
	//    DirMap = (vptr)PAllocPages;
	//    Mem_Set(DirMap, -1, 1024);
	//    Mem_Set(DirMap->Dirs, 0, sizeof(palloc_dir*) * 256);

	// 'Free' the memory that's available
	//    for(u32 I = 0; I < MemoryMapDescriptorCount; I++) {
	//       efi_memory_descriptor *Descriptor = (vptr)((u08*)MemoryMap +
	//       MemoryMapDescriptorSize*I);

	// switch(Descriptor->Type) {
	//    case EFI_MemoryType_Reserved:
	//    case EFI_MemoryType_LoaderData:
	//    case EFI_MemoryType_RuntimeServicesCode:
	//    case EFI_MemoryType_RuntimeServicesData:
	//    case EFI_MemoryType_Unusable:
	//    case EFI_MemoryType_ACPIReclaim:
	//    case EFI_MemoryType_ACPIMemoryNVS:
	//    case EFI_MemoryType_MappedIO:
	//    case EFI_MemoryType_MappedIOPortSpace:
	//    case EFI_MemoryType_PalCode:
	//    case EFI_MemoryType_Unaccepted:
	//    default:
	//       break;

	// case EFI_MemoryType_LoaderCode:
	// case EFI_MemoryType_BootServicesCode:
	// case EFI_MemoryType_BootServicesData:
	// case EFI_MemoryType_Conventional:
	// case EFI_MemoryType_Persistent: {
	//    FreePhysicalPageRange((pptr)Descriptor->PhysicalStart,
	//    Descriptor->PageCount);
	//    FreeVirtualPageRange(Descriptor->VirtualStart,
	//    Descriptor->PageCount, TRUE);
	// } break;
	// }
	// }

	Status		   = Serial_Init(38400, &Global.SerialPort);
	u16 SerialPort = Global.SerialPort;
	if (Status == ST_Success) Global.Flags |= HW_HasSerial;

	acpi ACPI = InitACPI(RSDP);
	InitAPIC(ACPI);

	GDT_Init();

	// TODO: Reclaim the ACPI data

	idt IDT;
	IDT_Init(&IDT);

	// TODO: Pluggable drivers?
	// TODO: Handle when certain devices are unavailable

	pci PCI;
	if (PRBIP) {
		PCI = PCI_Init();
		// PCI_GetDeviceMemory(PCI_Serial, PCI_Serial_USB, PCI_Serial_USB_XHCI);
	}

	// TODO: Definitely need a better PCI system
	if (PCI.XHCI & 0x80000000) {
		u08 B = (PCI.XHCI >> 16) & 0xFF;
		u08 D = (PCI.XHCI >> 8) & 0xFF;
		u08 F = (PCI.XHCI >> 0) & 0xFF;
		u64 BAR0 =
			PCI_Read32(B, D, F, OFFSETOF(pci_header, Type0.BaseAddress0));
		u64 BAR1 =
			PCI_Read32(B, D, F, OFFSETOF(pci_header, Type0.BaseAddress1));
		u64 Address = (BAR1 << 32) | BAR0;

		// PCI_EnableMSI(B, D, F);
	}

	EnableInterrupts();

	c08 Buffer[64];

	Serial_Write(SerialPort, "Hello! Testing the serial output\r\n");

	pptr MaxBase  = 0;
	u64	 MaxPages = 0;
	Serial_Write(
		SerialPort,
		"\r\nType\t    PStart\t    VStart\tPage Count\r\n"
	);
	for (u32 I = 0; I < MemoryMapDescriptorCount; I++) {
		efi_memory_descriptor *Descriptor =
			(vptr) ((u08 *) MemoryMap + MemoryMapDescriptorSize * I);

		if (Descriptor->PageCount > MaxPages) {
			MaxPages = Descriptor->PageCount;
			MaxBase	 = (u64) Descriptor->PhysicalStart;
		}

		Serial_Write(SerialPort, U64_ToStr(Buffer, Descriptor->Type, 16));
		Serial_Write(SerialPort, "\t");
		Serial_Write(
			SerialPort,
			U64_ToStrP(Buffer, (u64) Descriptor->PhysicalStart, 16, 10, ' ')
		);
		Serial_Write(SerialPort, "\t");
		Serial_Write(
			SerialPort,
			U64_ToStrP(Buffer, (u64) Descriptor->VirtualStart, 16, 10, ' ')
		);
		Serial_Write(SerialPort, "\t");
		Serial_Write(
			SerialPort,
			U64_ToStrP(Buffer, Descriptor->PageCount, 16, 10, ' ')
		);
		Serial_Write(SerialPort, "\r\n");
	}

	// Finish setting up the memory allocators by freeing unused memory
	{
		Assert(MaxPages > 0);

		PMapBase = MaxBase;
		if (MaxPages > 0x2000) MaxPages = 0x2000;

		vptr AddrS = (vptr) PMapBase;
		vptr AddrE = (vptr) (PMapBase + ((MaxPages - 1) << 12));

		vmap_path VPathS = GetVMapPathFromAddr(AddrS);
		vmap_path VPathE = GetVMapPathFromAddr(AddrE);
		pmap_path PPathS = GetPMapPathFromVMapPath(VPathS);
		pmap_path PPathE = GetPMapPathFromVMapPath(VPathE);

		ClearPMapLeafRange(PPathS, PPathE);
		UnmapPageLevel(VPathS, VPathE, 3 + VPathS.HasLvl5, FALSE);

		SetCR3(GetCR3());
	}

	Serial_Write(SerialPort, "CR0: ");
	Serial_Write(SerialPort, U64_ToStr(Buffer, GetCR0(), 16));
	Serial_Write(SerialPort, "\r\nCR3: ");
	Serial_Write(SerialPort, U64_ToStr(Buffer, GetCR3(), 16));
	Serial_Write(SerialPort, "\r\nCR4: ");
	Serial_Write(SerialPort, U64_ToStr(Buffer, GetCR4(), 16));
	Serial_Write(SerialPort, "\r\n");

	__asm__("hlt");

	return EFI_Status_Success;
}
