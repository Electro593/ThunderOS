/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                         **
**  Author: Aria Seiler                                                    **
**                                                                         **
**  This program is in the public domain. There is no implied warranty,    **
**  so use it at your own risk.                                            **
**                                                                         **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <shared.h>
#include <kernel/elf.h>

// CREDIT: POSIX-UEFI
asm (
"        .align 4                  \n"
"        .globl _start             \n"
"                                  \n"
"_start: lea ImageBase(%rip), %rdi \n"
"        lea _DYNAMIC(%rip), %rsi  \n"
"        call EFI_Entry            \n"
"        ret                       \n"
"                                  \n"
"  /* Fake a relocation record */  \n"
"        .data                     \n"
"dummy:  .long 0                   \n"
"        .section .reloc, \"a\"    \n"
"                                  \n"
"label1: .long   dummy-label1      \n"
"        .long   10                \n"
"        .word 0                   \n"
"                                  \n"
"        .text                     \n"
);

asm (
"        .globl setjmp         \n"
"                              \n"
"setjmp: pop  %rsi             \n"
"        movq %rbx, 0x00(%rdi) \n"
"        movq %rsp, 0x08(%rdi) \n"
"        push %rsi             \n"
"        movq %rbp, 0x10(%rdi) \n"
"        movq %r12, 0x18(%rdi) \n"
"        movq %r13, 0x20(%rdi) \n"
"        movq %r14, 0x28(%rdi) \n"
"        movq %r15, 0x30(%rdi) \n"
"        movq %rsi, 0x38(%rdi) \n"
"        xor  %rax, %rax       \n"
"        ret                   \n"
);

asm (
"         .globl longjmp        \n"
"                               \n"
"longjmp: movl %esi, %eax       \n"
"         movq 0x00(%rdi), %rbx \n"
"         movq 0x08(%rdi), %rsp \n"
"         movq 0x10(%rdi), %rbp \n"
"         movq 0x18(%rdi), %r12 \n"
"         movq 0x20(%rdi), %r13 \n"
"         movq 0x28(%rdi), %r14 \n"
"         movq 0x30(%rdi), %r15 \n"
"         xor %rdx, %rdx        \n"
"         mov $1, %rcx          \n"
"         cmp %rax, %rdx        \n"
"         cmove %rcx, %rax      \n"
"         jmp *0x38(%rdi)       \n"
);

#define INCLUDE_HEADER
    #include <kernel/efi.h>
    #include <drivers/acpi.c>
#undef INCLUDE_HEADER

typedef u32 (*kernel_entry)(rsdp *RSDP, efi_graphics_output_protocol *GOP);

internal void
U64_ToStr(c16 *Buffer, u64 N, u32 Radix) {
    persist c08 Chars[64] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-_";
    
    u64 M = N;
    u32 Len = 1;
    while(M /= Radix) Len++;
    
    if(Radix == 2) {
        Buffer[0] = '0';
        Buffer[1] = 'b';
        Len += 2;
    } else if(Radix == 8) {
        Buffer[0] = '0';
        Len += 1;
    } else if(Radix == 16) {
        Buffer[0] = '0';
        Buffer[1] = 'x';
        Len += 2;
    }
    
    Buffer[Len--] = 0;
    
    do {
        u16 Digit = N % Radix;
        Buffer[Len--] = (c16)Chars[Digit];
    } while(N /= Radix);
}

external efi_status
EFI_Entry(u64 LoadBase,
          elf64_dynamic *Dynamics,
          efi_system_table *SystemTable,
          efi_handle ImageHandle)
{
    UNUSED(LoadBase);
    UNUSED(Dynamics);
    
    efi_status Status;
    efi_boot_services *BootServices = SystemTable->BootServices;
    
    #define ASSERT(Expression, Message) \
        do { \
            if(!(Expression)) { \
                SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, Message); \
                Status = EFI_Status_Aborted; \
                goto _error; \
            } \
        } while(0)
    #define SASSERT(DesiredStatus, Message) \
        ASSERT(DesiredStatus == Status, Message)
    
    SystemTable->ConsoleOut->Reset(SystemTable->ConsoleOut, FALSE);
    
    efi_loaded_image_protocol *LoadedImage = NULL;
    Status = BootServices->HandleProtocol(ImageHandle, &EFI_GUID_LOADED_IMAGE_PROTOCOL, (vptr*)&LoadedImage);
    SASSERT(EFI_Status_Success, u"ERROR: Could not load LoadedImage Protocol\r\n");
    
    c16 Buffer[35];
    U64_ToStr(Buffer, (u64)LoadedImage->ImageBase, 16);
    SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut,
        u"NOTE: Loader image base is at ");
    SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut,
        Buffer);
    SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut,
        u"\r\n");
    
    //
    // Load the kernel
    //
    efi_simple_file_system_protocol *SFSP = 0;
    Status = BootServices->HandleProtocol(LoadedImage->DeviceHandle, &EFI_GUID_SIMPLE_FILE_SYSTEM_PROTOCOL, (vptr)&SFSP);
    SASSERT(EFI_Status_Success, u"ERROR: Could not load SimpleFileSystem Protocol\r\n");
    
    efi_file_protocol *Volume = 0;
    Status = SFSP->OpenVolume(SFSP, &Volume);
    SASSERT(EFI_Status_Success, u"ERROR: Could not open EFI volume\r\n");
    
    efi_file_protocol *FileHandle = 0;
    Status = Volume->Open(Volume, &FileHandle, L"\\kernel", EFI_FileMode_Read, 0);
    SASSERT(EFI_Status_Success, u"ERROR: Could not open kernel file\r\n");
    
    u64 FileInfoSize = 0;
    Status = FileHandle->GetInfo(FileHandle, &EFI_GUID_FILE_INFO, &FileInfoSize, NULL);
    SASSERT(EFI_Status_BufferTooSmall, u"ERROR: Could not get kernel file size\r\n");
    
    efi_file_info *FileInfo = 0;
    Status = BootServices->AllocatePool(EFI_MemoryType_LoaderData, FileInfoSize, (vptr*)&FileInfo);
    SASSERT(EFI_Status_Success, u"ERROR: Could not allocate kernel file info\r\n");
    Status = FileHandle->GetInfo(FileHandle, &EFI_GUID_FILE_INFO, &FileInfoSize, FileInfo);
    SASSERT(EFI_Status_Success, u"ERROR: Could not get kernel file info\r\n");
    u64 FileSize = FileInfo->FileSize;
    Status = BootServices->FreePool(FileInfo);
    SASSERT(EFI_Status_Success, u"ERROR: Could not free kernel file info\r\n");
    
    u08 *FileData = 0;
    Status = BootServices->AllocatePool(EFI_MemoryType_LoaderData, FileSize, (vptr*)&FileData);
    SASSERT(EFI_Status_Success, u"ERROR: Could not allocate kernel file\r\n");
    Status = FileHandle->Read(FileHandle, &FileSize, FileData);
    SASSERT(EFI_Status_Success, u"ERROR: Could not read kernel file\r\n");
    
    Status = FileHandle->Close(FileHandle);
    SASSERT(EFI_Status_Success, u"ERROR: Could not close kernel file\r\n");
    
    elf64_header *ELFHeader = (vptr)FileData;
    // TODO Validate the header
    
    elf64_program_header *ProgramHeaders = (vptr)(FileData + ELFHeader->ProgramHeaderOffset);
    u64 MinAddress = U64_MAX;
    u64 MaxAddress = 0;
    for(u32 I = 0; I < ELFHeader->ProgramHeaderCount; I++) {
        elf64_program_header *Header = ProgramHeaders+I;
        if(Header->Type == ELF_ProgramHeaderType_Load) {
            if(MinAddress > Header->VirtualAddress)
                MinAddress = Header->VirtualAddress;
            if(MaxAddress < Header->VirtualAddress+Header->SizeInMemory)
                MaxAddress = Header->VirtualAddress+Header->SizeInMemory;
        }
    }
    
    u64 PageCount = (MaxAddress-MinAddress + PAGE_SIZE-1) / PAGE_SIZE;
    // Status = BootServices->AllocatePages(EFI_AllocateType_AnyPages, EFI_MemoryType_LoaderData, PageCount, (vptr*)&MinAddress);
    MinAddress = 4096;
    Status = BootServices->AllocatePages(EFI_AllocateType_Address, EFI_MemoryType_LoaderData, PageCount, (vptr*)&MinAddress);
    SASSERT(EFI_Status_Success, u"ERROR: Could not allocate pages for kernel\r\n");
    
    for(u32 I = 0; I < ELFHeader->ProgramHeaderCount; I++) {
        elf64_program_header *Header = ProgramHeaders+I;
        if(Header->Type == ELF_ProgramHeaderType_Load) {
            u64 *Dest64, *Src64;
            u08 *Dest08, *Src08;
            Header->SizeInMemory -= Header->SizeInFile;
            
            Dest64 = (vptr)(Header->VirtualAddress + MinAddress);
            Src64 = (vptr)(FileData + Header->Offset);
            while(Header->SizeInFile >= 8) {
                *Dest64++ = *Src64++;
                Header->SizeInFile -= 8;
            }
            Dest08 = (u08*)Dest64;
            Src08 = (u08*)Src64;
            while(Header->SizeInFile) {
                *Dest08++ = *Src08++;
                Header->SizeInFile--;
            }
            while(Header->SizeInMemory && ((u64)Dest08 & 0b00111111)) {
                *Dest08++ = 0;
                Header->SizeInMemory--;
            }
            Dest64 = (u64*)Dest08;
            while(Header->SizeInMemory >= 8) {
                *Dest64++ = 0;
                Header->SizeInMemory -= 8;
            }
            Dest08 = (u08*)Dest64;
            while(Header->SizeInMemory) {
                *Dest08++ = 0;
                Header->SizeInMemory--;
            }
        }
    }
    
    u64 KernelEntryAddress = MinAddress + ELFHeader->Entry;
    Status = BootServices->FreePool(FileData);
    SASSERT(EFI_Status_Success, u"ERROR: Could not free kernel file\r\n");
    
    U64_ToStr(Buffer, KernelEntryAddress, 16);
    SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut,
        u"NOTE: Kernel entry point is at ");
    SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut,
        Buffer);
    SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut,
        u"\n");
    
    
    
    #if 0
    //
    // Memory
    //
    vptr MemBase;
    u64 StackSize = 64 * 1024 * 1024;
    Status = BootServices->AllocatePool(LoadedImage->ImageDataType, StackSize, &MemBase);
    Assert(Status == EFI_Status_Success);
    Context.Stack = Linear_Init(MemBase, StackSize);
    Context.Allocate = Linear_Allocate;
    
    u64 MemoryMapSize, MemoryDescriptorSize;
    Status = BootServices->GetMemoryMap(&MemoryMapSize, NULL, NULL, &MemoryDescriptorSize, NULL);
    efi_memory_descriptor *MemoryMap = Context.Allocate(MemoryMapSize);
    Status = BootServices->GetMemoryMap(&MemoryMapSize, MemoryMap, NULL, NULL, NULL);
    u32 MemoryDescriptorCount = MemoryMapSize / MemoryDescriptorSize;
    
    // PageDirPtrTbl[0] = (u64)PageDir | 1
    // PageDir[0] = 0b10000011;
    // for(u32 I = 0; I < 1024; I++) {
    //     PageDirectory[I] = 0x00000002;
    // }
    // for(u32 I = 0; I < 1024; I++) {
    //     PageTable[I] = (I * 0x1000) | 3;
    // }
    // PageDirectory[0] = (u32)(u64)(u32*)PageTable | 3;
    
    //
    // Files
    //
    efi_file_protocol *Volume = NULL;
    efi_simple_file_system_protocol *SFSP = NULL;
    Status = BootServices->HandleProtocol(LoadedImage->DeviceHandle, &EFI_GUID_SIMPLE_FILE_SYSTEM_PROTOCOL, (vptr)&SFSP);
    Assert(Status == EFI_Status_Success);
    Status = SFSP->OpenVolume(SFSP, &Volume);
    Assert(Status == EFI_Status_Success);
    
    vptr TTFData;
    efi_file_protocol *FileHandle;
    u64 BufferSize;
    efi_file_info *FileInfo = Context.Allocate(sizeof(efi_file_info));
    Status = Volume->Open(Volume, &FileHandle, L"\\assets\\cour.ttf", EFI_FileMode_Read, 0);
    Assert(Status == EFI_Status_Success);
    Status = FileHandle->GetInfo(FileHandle, &EFI_GUID_FILE_INFO, &BufferSize, FileInfo);
    if(Status == EFI_Status_BufferTooSmall) {
        FileInfo = Context.Allocate(BufferSize);
        Status = FileHandle->GetInfo(FileHandle, &EFI_GUID_FILE_INFO, &BufferSize, FileInfo);
    }
    TTFData = Context.Allocate(FileInfo->FileSize);
    Status = FileHandle->Read(FileHandle, &FileInfo->FileSize, TTFData);
    Assert(Status == EFI_Status_Success);
    Status = FileHandle->Close(FileHandle);
    Assert(Status == EFI_Status_Success);
    u64 FontFileSize;
    vptr FontFile;
    bitmap_header BitmapHeaderOut;
    CreateFontFile(TTFData, &FontFile, &FontFileSize, 28, &BitmapHeaderOut);
    Status = Volume->Open(Volume, &FileHandle, u"\\assets\\cour.font", EFI_FileMode_Create|EFI_FileMode_Read|EFI_FileMode_Write, 0);
    Status = FileHandle->Write(FileHandle, &FontFileSize, FontFile);
    Status = FileHandle->Close(FileHandle);
    
    
    //
    // Renderer and Terminal
    //
    software_renderer Renderer = {0};
    Renderer.Format = GOP->Mode->Info->PixelFormat == EFI_GraphicsPixelFormat_BlueGreenRedReserved8BitPerColor ? PixelFormat_BGRX_8 : PixelFormat_RGBX_8;
    Renderer.Size = (v2u32){GOP->Mode->Info->HorizontalResolution,
                            GOP->Mode->Info->VerticalResolution};
    u64 FramebufferSize = Renderer.Size.X * Renderer.Size.Y * 4;
    u32 *Framebuffer = Context.Allocate(FramebufferSize);
    Renderer.Framebuffer = Framebuffer;
    Renderer.Pitch = 4 * GOP->Mode->Info->PixelsPerScanLine;
    Renderer.BackgroundColor = V4u08(0, 0, 0, 0);
    
    
    terminal Terminal = InitTerminal(200, 4000, FontFile, Renderer.Size);
    Context.Terminal = &Terminal;
    c08 Text[] = "--------Terminal Test--------\nabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ \t 1234567890`~!@#$%^&*()-_=+[{]}\\|;:'\",<.>/?\nNewline: [\n]\nTab: [\t]\n-----------------------------\n\n";
    WriteToTerminal(&Terminal, Text, sizeof(Text)-1);
    //  At size 32: 
    //    --------Terminal Test--------
    //    abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVW
    //    XYZ      1234567890`~!@#$%^&*()-
    //    _=+[{]}\\|;:'\",<.>/?
    //    Newline: [
    //    ]
    //    Tab: [    ]
    //    -----------------------------
    //    
    DrawTerminal(Framebuffer, &Terminal);
    
    
    
    WriteToTerminal(&Terminal, "Image base is at: 0x", 0);
    c08 Buffer[32];
    u32 Index = sizeof(Buffer);
    Buffer[--Index] = 0;
    u64 Value = (u64)LoadedImage->ImageBase;
    do {
        u32 Digit = Value % 16;
        if(Digit < 10) Buffer[--Index] = Digit + '0';
        else           Buffer[--Index] = Digit - 10 + 'A';
        Value /= 16;
    } while(Value > 0);
    WriteToTerminal(&Terminal, Buffer+Index, 0);
    WriteToTerminal(&Terminal, "\n", 1);
    Mem_Set(Framebuffer, 0, FramebufferSize);
    DrawTerminal(Framebuffer, &Terminal);
    GOP->Blt(GOP, (efi_graphics_output_blt_pixel*)Framebuffer, EFI_GraphicsOutputBltOperation_BufferToVideo, 0, 0, 0, 0, Renderer.Size.X, Renderer.Size.Y, 0);
    
    for(u32 I = 0; I < MemoryDescriptorCount; I++) {
        efi_memory_descriptor *Descriptor = (vptr)((u08*)MemoryMap+I*MemoryDescriptorSize);
        
        WriteToTerminal(&Terminal, "Type: ", 0);
        Index = sizeof(Buffer);
        Buffer[--Index] = 0;
        Value = Descriptor->Type;
        do {
            u32 Digit = Value % 16;
            if(Digit < 10) Buffer[--Index] = Digit + '0';
            else           Buffer[--Index] = Digit - 10 + 'A';
            Value /= 16;
        } while(Value > 0);
        WriteToTerminal(&Terminal, Buffer+Index, 0);
        
        WriteToTerminal(&Terminal, " :: Attribute: ", 0);
        Index = sizeof(Buffer);
        Buffer[--Index] = 0;
        Value = Descriptor->Attribute;
        do {
            u32 Digit = Value % 16;
            if(Digit < 10) Buffer[--Index] = Digit + '0';
            else           Buffer[--Index] = Digit - 10 + 'A';
            Value /= 16;
        } while(Value > 0);
        WriteToTerminal(&Terminal, Buffer+Index, 0);
        WriteToTerminal(&Terminal, "\n", 0);
        
        Mem_Set(Framebuffer, 0, FramebufferSize);
        DrawTerminal(Framebuffer, &Terminal);
        GOP->Blt(GOP, (efi_graphics_output_blt_pixel*)Framebuffer, EFI_GraphicsOutputBltOperation_BufferToVideo, 0, 0, 0, 0, Renderer.Size.X, Renderer.Size.Y, 0);
    }
    
    
    
    // InitPS2Controller(ACPI);
    // DetectPS2Device();
    
    
    
    // u08 Data;
    // while(!PS2_ReceiveData(&Data));
    
    
    // asm ("int $3");
    
    
    Mem_Set(Framebuffer, 0, FramebufferSize);
    WriteToTerminal(&Terminal, "\n----  END  ----", 1);
    DrawTerminal(Framebuffer, &Terminal);
    GOP->Blt(GOP, (efi_graphics_output_blt_pixel*)Framebuffer, EFI_GraphicsOutputBltOperation_BufferToVideo, 0, 0, 0, 0, Renderer.Size.X, Renderer.Size.Y, 0);
    
    
    BootServices->FreePool(Context.Stack);
    
    #endif
    
    rsdp *RSDP = NULL;
    for(u32 I = 0; I < SystemTable->ConfigTableEntryCount; I++) {
        efi_configuration_table ConfigTable = SystemTable->ConfigTable[I];
        if(*(u64*)&ConfigTable.VendorGuid.Data1 == *(u64*)&EFI_GUID_ACPI_TABLE_2_0.Data1 &&
           *(u64*)&ConfigTable.VendorGuid.Data4 == *(u64*)&EFI_GUID_ACPI_TABLE_2_0.Data4) {
            RSDP = ConfigTable.VendorTable;
        }
    }
    
    
    efi_graphics_output_protocol *GOP;
    efi_guid GOPGUID = EFI_GUID_GRAPHICS_OUTPUT_PROTOCOL;
    Status = BootServices->LocateProtocol(&GOPGUID, NULL, (vptr*)&GOP);
    SASSERT(EFI_Status_Success, u"ERROR: Could not load GraphicsOutput Protocol\r\n");
    
    
    u64 MemoryMapKey,MemoryMapSize;
    Status = BootServices->GetMemoryMap(&MemoryMapSize, NULL, &MemoryMapKey, NULL, NULL);
    Status = BootServices->ExitBootServices(ImageHandle, MemoryMapKey);
    SASSERT(EFI_Status_Success, u"ERROR: Could not exit boot services\n");
    
    Status = ((kernel_entry)KernelEntryAddress)(RSDP, GOP);
    return Status;
    
    
    #undef SASSERT
    #undef ASSERT
    
    _error:
    b08 Wait = TRUE;
    while(Wait)
        asm ("pause");
    
    return Status;
}