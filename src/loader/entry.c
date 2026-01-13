/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                           **
**  Author: Aria Seiler                                                      **
**                                                                           **
**  This program is in the public domain. There is no implied warranty,      **
**  so use it at your own risk.                                              **
**                                                                           **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// CREDIT: POSIX-UEFI
__asm__(
	".align 4							\n"
	"									\n"
	".globl _start						\n"
	"_start:							\n"
	"	lea rdi, [rip+ImageBase]		\n"
	"	lea rsi, [rip+_DYNAMIC]			\n"
	"	sub rsp, 8						\n"
	"	call EFI_Entry					\n"
	"	ret								\n"
	"									\n"
	"  /* Fake a relocation record */	\n"
	".data								\n"
	"									\n"
	"dummy:								\n"
	"	.long 0							\n"
	"	.section .reloc, \"a\"			\n"
	"									\n"
	"label1:							\n"
	"	.long dummy-label1				\n"
	"	.long 10						\n"
	"	.word 0							\n"
	"									\n"
	".text								\n"
);

#include <shared.h>
#include "elf.h"
#include "efi.h"

typedef u32 (*kernel_entry)(
	void							*RSDP,
	efi_graphics_output_protocol	*GOP,
	efi_pci_root_bridge_io_protocol *PRBIP,
	efi_simple_file_system_protocol *SFSP,
	efi_memory_descriptor			*MemoryMap,
	u64								 MemoryMapDescriptorSize,
	u32								 MemoryMapDescriptorCount
);

static void
U64_ToStr(c16 *Buffer, u64 N, u32 Radix)
{
	static c08 Chars[64] =
		"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-_";

	u64 M	= N;
	u32 Len = 1;
	while (M /= Radix) Len++;

	if (Radix == 2) {
		Buffer[0]  = '0';
		Buffer[1]  = 'b';
		Len		  += 2;
	} else if (Radix == 8) {
		Buffer[0]  = '0';
		Len		  += 1;
	} else if (Radix == 16) {
		Buffer[0]  = '0';
		Buffer[1]  = 'x';
		Len		  += 2;
	}

	Buffer[Len--] = 0;

	do {
		u16 Digit	  = N % Radix;
		Buffer[Len--] = (c16) Chars[Digit];
	} while (N /= Radix);
}

internal s08
_strncmp(c08 *A, c08 *B, u32 N)
{
	while (N && *A == *B) N--, A++, B++;
	if (N == 0) return 0;
	if (*A < *B) return -1;
	return 1;
}

internal s08
_strcmp(c08 *A, c08 *B)
{
	while (*A && *B && *A == *B) A++, B++;
	if (*A == *B) return 0;
	if (*A < *B) return -1;
	return 1;
}

static void
MemCpy(void *Dest, void *Source, usize Size)
{
	u08 *D = Dest, *S = Source;
	for (usize I = 0; I < Size; I++) *D++ = *S++;
}

static void
MemSet(void *Dest, u08 Byte, usize Size)
{
	u08 *D = Dest;
	for (usize I = 0; I < Size; I++) *D++ = Byte;
}

external efi_status
EFI_Entry(
	u64				  LoadBase,
	elf64_dynamic	 *Dynamics,
	efi_system_table *SystemTable,
	efi_handle		  ImageHandle
)
{
	UNUSED(LoadBase);
	UNUSED(Dynamics);

	efi_status		   Status;
	efi_boot_services *BootServices = SystemTable->BootServices;

#define ASSERT(Expression, Message)                                            \
  do {                                                                         \
    if (!(Expression)) {                                                       \
      SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, Message); \
      Status = EFI_Status_Aborted;                                             \
      goto _error;                                                             \
    }                                                                          \
  } while (0)
#define SASSERT(DesiredStatus, Message) ASSERT(DesiredStatus == Status, Message)

	SystemTable->ConsoleOut->Reset(SystemTable->ConsoleOut, FALSE);

	efi_loaded_image_protocol *LoadedImage = NULL;
	Status								   = BootServices->HandleProtocol(
		ImageHandle,
		&EFI_GUID_LOADED_IMAGE_PROTOCOL,
		(vptr *) &LoadedImage
	);
	SASSERT(
		EFI_Status_Success,
		u"ERROR: Could not load LoadedImage Protocol\r\n"
	);

	c16 Buffer[128];
	U64_ToStr(Buffer, (u64) LoadedImage->ImageBase, 16);
	SystemTable->ConsoleOut->OutputString(
		SystemTable->ConsoleOut,
		u"NOTE: Loader image base is at "
	);
	SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, Buffer);
	SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, u"\r\n");

	//
	// Load the kernel
	//
	efi_simple_file_system_protocol *SFSP;
	Status = BootServices->HandleProtocol(
		LoadedImage->DeviceHandle,
		&EFI_GUID_SIMPLE_FILE_SYSTEM_PROTOCOL,
		(vptr) &SFSP
	);
	SASSERT(
		EFI_Status_Success,
		u"ERROR: Could not load SimpleFileSystem Protocol\r\n"
	);

	efi_file_protocol *Volume = 0;
	Status					  = SFSP->OpenVolume(SFSP, &Volume);
	SASSERT(EFI_Status_Success, u"ERROR: Could not open EFI volume\r\n");

	efi_file_protocol *FileHandle = 0;
	Status =
		Volume->Open(Volume, &FileHandle, L"\\kernel", EFI_FileMode_Read, 0);
	SASSERT(EFI_Status_Success, u"ERROR: Could not open kernel file\r\n");

	u64 FileInfoSize = 0;
	Status =
		FileHandle
			->GetInfo(FileHandle, &EFI_GUID_FILE_INFO, &FileInfoSize, NULL);
	SASSERT(
		EFI_Status_BufferTooSmall,
		u"ERROR: Could not get kernel file size\r\n"
	);

	efi_file_info *FileInfo = 0;
	Status					= BootServices->AllocatePool(
		 EFI_MemoryType_LoaderData,
		 FileInfoSize,
		 (vptr *) &FileInfo
	 );
	SASSERT(
		EFI_Status_Success,
		u"ERROR: Could not allocate kernel file info\r\n"
	);
	Status =
		FileHandle
			->GetInfo(FileHandle, &EFI_GUID_FILE_INFO, &FileInfoSize, FileInfo);
	SASSERT(EFI_Status_Success, u"ERROR: Could not get kernel file info\r\n");
	u64 FileSize = FileInfo->FileSize;
	Status		 = BootServices->FreePool(FileInfo);
	SASSERT(EFI_Status_Success, u"ERROR: Could not free kernel file info\r\n");

	u08 *FileData = 0;
	Status		  = BootServices->AllocatePool(
		   EFI_MemoryType_LoaderData,
		   FileSize,
		   (vptr *) &FileData
	   );
	SASSERT(EFI_Status_Success, u"ERROR: Could not allocate kernel file\r\n");
	Status = FileHandle->Read(FileHandle, &FileSize, FileData);
	SASSERT(EFI_Status_Success, u"ERROR: Could not read kernel file\r\n");

	Status = FileHandle->Close(FileHandle);
	SASSERT(EFI_Status_Success, u"ERROR: Could not close kernel file\r\n");

	elf64_header *ELFHeader = (vptr) FileData;

	// Find the address to map into
	void *ImageBase = 0;
	void *ImageTop	= 0;

	void *ProgHeaders = FileData + ELFHeader->ProgramHeaderOffset;
	for (usize I = 0; I < ELFHeader->ProgramHeaderCount; I++) {
		elf64_program_header *ProgHeader =
			ProgHeaders + I * ELFHeader->ProgramHeaderSize;

		if (ProgHeader->Type == ELF_ProgramHeaderType_Load) {
			if (!ImageBase) ImageBase = (void *) ProgHeader->VirtualAddress;

			ImageTop =
				(void *) ProgHeader->VirtualAddress + ProgHeader->SizeInMemory;
		}
	}

	usize KernelPages = ((usize) (ImageTop - ImageBase) >> 12) + 1;

	Status = BootServices->AllocatePages(
		EFI_AllocateType_Address,
		EFI_MemoryType_Conventional,
		KernelPages,
		(vptr *) &ImageBase
	);
	SASSERT(
		EFI_Status_Success,
		u"ERROR: Could not allocate kernel mapping\r\n"
	);

	void *KernelDynamics = NULL;
	for (usize I = 0; I < ELFHeader->ProgramHeaderCount; I++) {
		elf64_program_header *ProgHeader =
			ProgHeaders + I * ELFHeader->ProgramHeaderSize;

		if (ProgHeader->Type == ELF_ProgramHeaderType_Load) {
			void *Dest	   = (void *)ProgHeader->VirtualAddress;
			void *Src	   = FileData + ProgHeader->Offset;
			usize FileSize = ProgHeader->SizeInFile;
			usize MemSize  = ProgHeader->SizeInMemory;
			MemCpy(Dest, Src, FileSize);
			MemSet(Dest + FileSize, 0, MemSize - FileSize);
		} else if (ProgHeader->Type == ELF_ProgramHeaderType_Dynamic) {
			KernelDynamics = (void *)FileData + ProgHeader->Offset;
		}
	}
	ASSERT(KernelDynamics, u"ERROR: Missing dynamic segment\r\n");

	elf64_dynamic *Dynamic = KernelDynamics;
	usize		   RelaEnt = 0, RelaSize = 0, SymEnt = 0;
	void		  *RelaBase = 0;
	while (Dynamic->Tag != ELF_DynamicType_Null) {
		if (Dynamic->Tag == ELF_DynamicType_Rela)
			RelaBase = FileData + Dynamic->Address;
		else if (Dynamic->Tag == ELF_DynamicType_RelaEnt)
			RelaEnt = Dynamic->Value;
		else if (Dynamic->Tag == ELF_DynamicType_RelaSize)
			RelaSize = Dynamic->Value;
		else if (Dynamic->Tag == ELF_DynamicType_SymEnt)
			SymEnt = Dynamic->Value;
		Dynamic++;
	}
	ASSERT(
		RelaEnt && RelaSize && SymEnt && RelaBase,
		u"ERROR: Missing required dynamic sections\r\n"
	);

	usize RelaCount = RelaSize / RelaEnt;
	for (usize I = 0; I < RelaCount; I++) {
		elf64_relocation_addend *Rela = RelaBase + I * RelaEnt;

		usize SymbolIndex = Rela->Info >> 32;
		usize RelocType	  = Rela->Info & 0xFFFFFFFF;

		usize Addend = Rela->Addend;
		void *Target = ImageBase + Rela->Offset;

		switch (RelocType) {
			case ELF_RelocationType_AMD64_Rel:
				*(u64 *) Target = (usize) ImageBase + Rela->Addend;
				break;

			default: ASSERT(FALSE, u"ERROR: Unhandled relocation type\r\n");
		}
	}

	void *KernelEntryAddress = (void *)ELFHeader->Entry;

	Status = BootServices->FreePool(FileData);
	SASSERT(EFI_Status_Success, u"ERROR: Could not free kernel file\r\n");

	U64_ToStr(Buffer, (usize) KernelEntryAddress, 16);
	SystemTable->ConsoleOut->OutputString(
		SystemTable->ConsoleOut,
		u"NOTE: Kernel entry point is at "
	);
	SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, Buffer);
	SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, u"\r\n");

	SystemTable->ConsoleOut->OutputString(
		SystemTable->ConsoleOut,
		u"Finding RSDP...\r\n"
	);

	void *RSDP = NULL;
	for (u32 I = 0; I < SystemTable->ConfigTableEntryCount; I++) {
		efi_configuration_table ConfigTable = SystemTable->ConfigTable[I];
		if (*(u64 *) &ConfigTable.VendorGuid.Data1
				== *(u64 *) &EFI_GUID_ACPI_TABLE_2_0.Data1
			&& *(u64 *) &ConfigTable.VendorGuid.Data4
				   == *(u64 *) &EFI_GUID_ACPI_TABLE_2_0.Data4)
		{
			RSDP = ConfigTable.VendorTable;
		}
	}

	// TODO: Make these asserts handle instead of crashing

	efi_graphics_output_protocol *GOP;
	Status = BootServices->LocateProtocol(
		&EFI_GUID_GRAPHICS_OUTPUT_PROTOCOL,
		NULL,
		(vptr *) &GOP
	);
	SASSERT(
		EFI_Status_Success,
		u"ERROR: Could not load GraphicsOutput Protocol\r\n"
	);

	efi_pci_root_bridge_io_protocol *PRBIP;
	Status = BootServices->LocateProtocol(
		&EFI_GUID_PCI_ROOT_BRIDGE_IO_PROTOCOL,
		NULL,
		(vptr *) &PRBIP
	);

	SystemTable->ConsoleOut->OutputString(
		SystemTable->ConsoleOut,
		u"NOTE: Initilalizing memory map...\r\n"
	);

	efi_memory_descriptor *MemoryMap;
	u64 MemoryMapKey, MemoryMapSize = 0, MemoryMapDescriptorSize = 0;
	Status = BootServices->GetMemoryMap(
		&MemoryMapSize,
		NULL,
		NULL,
		&MemoryMapDescriptorSize,
		NULL
	);
	MemoryMapSize += 2 * sizeof(efi_memory_descriptor);
	Status		   = BootServices->AllocatePool(
		EFI_MemoryType_LoaderData,
		MemoryMapSize,
		(vptr *) &MemoryMap
	);
	SASSERT(
		EFI_Status_Success,
		u"ERROR: Could not allocate memory for memory map\r\n"
	);
	Status = BootServices->GetMemoryMap(
		&MemoryMapSize,
		MemoryMap,
		&MemoryMapKey,
		&MemoryMapDescriptorSize,
		NULL
	);
	SASSERT(EFI_Status_Success, u"ERROR: Could not get memory map\r\n");

	Status = BootServices->ExitBootServices(ImageHandle, MemoryMapKey);
	SASSERT(EFI_Status_Success, u"ERROR: Could not exit boot services\r\n");

	Status = ((kernel_entry) KernelEntryAddress)(
		RSDP,
		GOP,
		PRBIP,
		SFSP,
		MemoryMap,
		MemoryMapDescriptorSize,
		MemoryMapSize / MemoryMapDescriptorSize
	);
	return Status;

#undef SASSERT
#undef ASSERT

_error:
	b08 Wait = TRUE;
	while (Wait) asm("pause");

	return Status;
}
