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

internal void Convert(vptr Out, type TypeOut, vptr In, type TypeIn);

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

extern u08  PortIn08(u16 Address);
extern u16  PortIn16(u16 Address);
extern u32  PortIn32(u16 Address);
extern void PortOut08(u16 Address, u08 Data);
extern void PortOut16(u16 Address, u16 Data);
extern void PortOut32(u16 Address, u32 Data);

typedef enum io_ports {
    Port_PS2_Data    = IN OUT 0x60,
    Port_PS2_Status  = IN     0x64,
    Port_PS2_Command = OUT    0x64,
} io_ports;

// internal void _Breakpoint(void) { }
// internal void KernelError(c08 *File, u32 Line, c08 *Expression);

#include <kernel/efi.h>
#include <util/intrin.h>
#include <util/mem.c>
#include <util/vector.c>
#include <util/str.c>
// #include <drivers/acpi.c>
// #include <drivers/ps2.c>
// #include <drivers/pcie.c>
#include <render/font.c>
#include <render/software.c>
#include <render/terminal.c>

internal void
Convert(vptr Out,
        type TypeOut,
        vptr In,
        type TypeIn)
{
    b08 IsUInt = FALSE;
    u64 UIntValue = 0;
    
    switch(TypeIn)
    {
        case Type_C08p: {
            c08 *C08p = *(c08**)In;
            u32 Length = Mem_BytesUntil(0, C08p);
            
            switch(TypeOut)
            {
                case Type_Str: {
                    str Result;
                    Result.Length = Mem_BytesUntil(0, C08p);
                    Result.Capacity = Result.Length;
                    Result.Data = Context.Allocate(Result.Length * sizeof(c16));
                    for(u32 I = 0; I < Length; ++I)
                        Result.Data[I] = (c16)C08p[I];
                    
                    *(str*)Out = Result;
                } break;
            }
        } break;
        
        case Type_U08: {
            UIntValue = (u64)*(u08*)In;
            IsUInt = TRUE;
        } break;
        case Type_U16: {
            UIntValue = (u64)*(u16*)In;
            IsUInt = TRUE;
        } break;
        case Type_U32: {
            UIntValue = (u64)*(u32*)In;
            IsUInt = TRUE;
        } break;
        case Type_U64: {
            UIntValue = *(u64*)In;
            IsUInt = TRUE;
        } break;
        
        NO_DEFAULT;
    }
    
    if(IsUInt)
    {
        switch(TypeOut)
        {
            case Type_Str:
            {
                str Result;
                Result.Capacity = 20; // Max length of unsigned 64-bit base 10
                Result.Data = Context.Allocate(Result.Capacity);
                
                u32 Index = Result.Capacity;
                do {
                    Index--;
                    Result.Data[Index] = (UIntValue % 10) + L'0';
                    UIntValue /= 10;
                } while(UIntValue > 0);
                
                Result.Length = Result.Capacity - Index;
                Result.Data += Index;
                
                *(str*)Out = Result;
            } break;
            
            NO_DEFAULT;
        }
    }
}

// internal void
// KernelError(c08 *File,
//             u32 Line,
//             c08 *Expression)
// {
//     if(Context.Renderer == NULL ||
//        Context.Terminal == NULL)
//     {
//         //TODO Uhhh... do something here
//         return;
//     }
    
//     c08 Buffer[32];
//     u32 Index = sizeof(Buffer);
//     u32 Value = Line;
//     Buffer[--Index] = 0;
//     do {
//         Buffer[--Index] = (Value % 10) + '0';
//         Value /= 10;
//     } while(Value > 0);
    
//     WriteToTerminal(Context.Terminal, "Error in ", 0);
//     WriteToTerminal(Context.Terminal, File, 0);
//     WriteToTerminal(Context.Terminal, ", on line ", 0);
//     WriteToTerminal(Context.Terminal, Buffer+Index, 0);
//     WriteToTerminal(Context.Terminal, ": ", 0);
//     WriteToTerminal(Context.Terminal, Expression, 0);
//     WriteToTerminal(Context.Terminal, "\n", 0);
//     DrawTerminal(Context.Renderer->Framebuffer, Context.Terminal);
    
//     // Context.GOP->Blt(Context.GOP, (efi_graphics_output_blt_pixel*)Context.Renderer->Framebuffer, EFI_GraphicsOutputBltOperation_BufferToVideo, 0, 0, 0, 0, Context.Renderer->Size.X, Context.Renderer->Size.Y, 0);
    
//     _Breakpoint();
// }

external efi_status
EFI_Entry(u64 LoadBase,
          elf64_dynamic *Dynamics,
          efi_system_table *SystemTable,
          efi_handle ImageHandle)
{
    efi_status Status;
    
    /* handle relocations */
    s32 RelTotalSize = 0;
    s32 RelEntrySize = 0;
    elf64_relocation *Rel = 0;
    for(u32 I = 0;
        Dynamics[I].Tag != DT_NULL;
        ++I)
    {
        switch(Dynamics[I].Tag)
        {
            case DT_RELA: {
                Rel = (elf64_relocation*)((u32)Dynamics[I].Address + LoadBase);
            } break;
            
            case DT_RELASZ: {
                RelTotalSize = Dynamics[I].Address;
            } break;
            
            case DT_RELAENT: {
                RelEntrySize = Dynamics[I].Address;
            } break;
        }
    }
    if(Rel && RelEntrySize)
    {
        while(RelTotalSize > 0)
        {
            if(ELF64_R_TYPE(Rel->Info) == R_X86_64_RELATIVE)
            {
                u64 *Address = (u64*)(LoadBase + Rel->Offset);
                *Address += LoadBase;
            }
            Rel = (elf64_relocation*)((u08*)Rel + RelEntrySize);
            RelTotalSize -= RelEntrySize;
        }
    }
    
    // Make sure SSE is enabled
    asm volatile (
    "movq %cr0, %rax  \n"
    "andb $0xF1, %al  \n"
    "movq %rax, %cr0  \n"
    "movq %cr4, %rax  \n"
    "orw $3 << 9, %ax \n"
    "mov %rax, %cr4   \n"
    );
    
    efi_loaded_image_protocol *LoadedImage = NULL;
    SystemTable->BootServices->HandleProtocol(ImageHandle, &EFI_GUID_LOADED_IMAGE_PROTOCOL, (vptr*)&LoadedImage);
    SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, L"Waiting for debugger...\n\r");
    
    // asm("int $3");
    
    SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, L"Debugger Connected!\n\r");
    SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, L"Welcome to ThunderOS.\n\r");
    
    Context.PrevContext = (context*)&Context.PrevContext;
    Context.Allocate = Stack_Allocate;
    
    vptr MemBase;
    u64 StackSize = 64 * 1024 * 1024;
    Status = SystemTable->BootServices->AllocatePool(LoadedImage->ImageDataType, StackSize, &MemBase);
    ASSERT(Status == EFI_Status_Success);
    u08 *MemCursor = MemBase;
    
    stack *Stack = Stack_Init(MemCursor, StackSize);
    Context.Stack = Stack;
    MemCursor += StackSize;
    
    
    // u64 MemoryMapSize = 0;
    // Status = SystemTable->BootServices->GetMemoryMap(&MemoryMapSize, NULL, NULL, NULL, NULL);
    // u64 MemoryMapKey, MemoryDescriptorSize;
    // efi_memory_descriptor *MemoryMap = Context.Allocate(MemoryMapSize);
    // Status = SystemTable->BootServices->GetMemoryMap(&MemoryMapSize, MemoryMap, &MemoryMapKey, &MemoryDescriptorSize, NULL);
    // u32 MemoryDescriptorCount = MemoryMapSize / MemoryDescriptorSize;
    // for(u32 DescriptorIndex = 0; DescriptorIndex < MemoryDescriptorCount; ++DescriptorIndex)
    // {
    //     efi_memory_descriptor Descriptor = MemoryMap[DescriptorIndex];
    // }
    
    
    // rsdp *RSDP = NULL;
    // {
    //     rsdp *RSDPVersion1 = NULL;
    //     efi_configuration_table *ConfigTable = SystemTable->ConfigTable;
    //     for(u32 Index = 0; Index < SystemTable->ConfigTableEntryCount; ++Index)
    //     {
    //         if(*(u64*)&ConfigTable->VendorGuid.Data1 == *(u64*)&EFI_GUID_ACPI_TABLE_1_0.Data1 &&
    //            *(u64*)&ConfigTable->VendorGuid.Data4 == *(u64*)&EFI_GUID_ACPI_TABLE_1_0.Data4) {
    //             RSDPVersion1 = ConfigTable->VendorTable;
    //             // Assert(RSDPVersion1->Revision == 0);
    //         }
            
    //         if(*(u64*)&ConfigTable->VendorGuid.Data1 == *(u64*)&EFI_GUID_ACPI_TABLE_2_0.Data1 &&
    //            *(u64*)&ConfigTable->VendorGuid.Data4 == *(u64*)&EFI_GUID_ACPI_TABLE_2_0.Data4) {
    //             RSDP = ConfigTable->VendorTable;
    //             // Assert(RSDP->Revision == 2);
    //         }
            
    //         ++ConfigTable;
    //     }
        
    //     if(RSDP == NULL)
    //         RSDP = RSDPVersion1;
    // }
    
    
    
    
    // TODO: Verify that ASSERT works and make a better one
    // TODO: Replace this with GPU stuff eventually
    efi_guid GOPGUID = EFI_GUID_GRAPHICS_OUTPUT_PROTOCOL;
    efi_graphics_output_protocol *GOP;
    // Context.GOP = GOP;
    Status = SystemTable->BootServices->LocateProtocol(&GOPGUID, NULL, (vptr*)&GOP);
    ASSERT(Status == EFI_Status_Success);
    
    efi_graphics_output_mode_information *Info;
    u32 NumModes, NativeMode;
    u64 SizeOfInfo;
    Status = GOP->QueryMode(GOP, (GOP->Mode == NULL) ? 0 : GOP->Mode->Mode, &SizeOfInfo, &Info);
    ASSERT(Status == EFI_Status_Success);
    NativeMode = GOP->Mode->Mode;
    NumModes = GOP->Mode->MaxMode;
    
    str String;
    SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, L"Querying graphics modes...\n\r");
    u32 Mode = NativeMode;
    for(u32 Index = 0;
        Index < NumModes;
        ++Index)
    {
        Status = GOP->QueryMode(GOP, Index, &SizeOfInfo, &Info);
        String = Printc(L"    Mode $u32$: Width $u32$, height $u32$, format $u32$ $c08*$\n\r",
            Index, Info->HorizontalResolution, Info->VerticalResolution, Info->PixelFormat,
            (Index == NativeMode) ? "(current)" : "");
        SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, String.Data);
        
    }
    
    software_renderer Renderer = {0};
    Renderer.Format = GOP->Mode->Info->PixelFormat == EFI_GraphicsPixelFormat_BlueGreenRedReserved8BitPerColor ? PixelFormat_BGRX_8 : PixelFormat_RGBX_8;
    Renderer.Size = (v2u32){GOP->Mode->Info->HorizontalResolution,
                            GOP->Mode->Info->VerticalResolution};
    u64 FramebufferSize = Renderer.Size.X * Renderer.Size.Y * 4;
    u32 *Framebuffer = Context.Allocate(FramebufferSize);
    Renderer.Framebuffer = Framebuffer;
    Renderer.Pitch = 4 * GOP->Mode->Info->PixelsPerScanLine;
    Renderer.BackgroundColor = V4u08(0, 0, 0, 0);
    // Context.Renderer = &Renderer;
    
    
    efi_simple_file_system_protocol *SFSP = NULL;
    SystemTable->BootServices->HandleProtocol(LoadedImage->DeviceHandle, &EFI_GUID_SIMPLE_FILE_SYSTEM_PROTOCOL, (vptr)&SFSP);
    efi_file_protocol *Volume = NULL;
    SFSP->OpenVolume(SFSP, &Volume);
    
    vptr TTFData;
    efi_file_protocol *FileHandle;
    u64 BufferSize;
    efi_file_info *FileInfo = Context.Allocate(sizeof(efi_file_info));
    Status = Volume->Open(Volume, &FileHandle, L"\\assets\\cour.ttf", EFI_FileMode_Read, 0);
    Status = FileHandle->GetInfo(FileHandle, &EFI_GUID_FILE_INFO, &BufferSize, FileInfo);
    if(Status == EFI_Status_BufferTooSmall) {
        FileInfo = Context.Allocate(BufferSize);
        Status = FileHandle->GetInfo(FileHandle, &EFI_GUID_FILE_INFO, &BufferSize, FileInfo);
    }
    TTFData = Context.Allocate(FileInfo->FileSize);
    Status = FileHandle->Read(FileHandle, &FileInfo->FileSize, TTFData);
    Status = FileHandle->Close(FileHandle);
    
    u64 FontFileSize;
    vptr FontFile;
    bitmap_header BitmapHeaderOut;
    CreateFontFile(TTFData, &FontFile, &FontFileSize, 32, &BitmapHeaderOut);
    Status = Volume->Open(Volume, &FileHandle, u"\\assets\\cour.font", EFI_FileMode_Create|EFI_FileMode_Read|EFI_FileMode_Write, 0);
    Status = FileHandle->Write(FileHandle, &FontFileSize, FontFile);
    Status = FileHandle->Close(FileHandle);
    
    c08 Text[] = "Hello, world!\n\t    Pleasant day it is outside, isn't it?\n\nabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890`~!@#$%^&*()-_=+[{]}\\|;:'\",<.>/?\nNewline: [\n]\nTab: [\t]\n";
    terminal Terminal = InitTerminal(100, 2000, FontFile, Renderer.Size);
    // Context.Terminal = &Terminal;
    WriteToTerminal(&Terminal, Text, sizeof(Text)-1);
    DrawTerminal(Framebuffer, &Terminal);
    GOP->Blt(GOP, (efi_graphics_output_blt_pixel*)Framebuffer, EFI_GraphicsOutputBltOperation_BufferToVideo, 0, 0, 0, 0, Renderer.Size.X, Renderer.Size.Y, 0);
    for(u32 Line = 0; Line < 30; Line++) {
        Mem_Set(Framebuffer, 0, FramebufferSize);
        
        c08 NewText[] = "Line --\n";
        NewText[5] = (Line / 10) + '0';
        NewText[6] = (Line % 10) + '0';
        WriteToTerminal(&Terminal, NewText, sizeof(NewText)-1);
        DrawTerminal(Framebuffer, &Terminal);
        
        GOP->Blt(GOP, (efi_graphics_output_blt_pixel*)Framebuffer, EFI_GraphicsOutputBltOperation_BufferToVideo, 0, 0, 0, 0, Renderer.Size.X, Renderer.Size.Y, 0);
    }
    
    // Assert(FALSE);
    
    // SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, L"Image base is at: 0x");
    // u32 Index = 32;
    // c16 Buffer[32];
    // Buffer[--Index] = 0;
    // u64 Value = (u64)LoadedImage->ImageBase;
    // do {
    //     u32 Digit = Value % 16;
    //     if(Digit < 10) Buffer[--Index] = Digit + L'0';
    //     else           Buffer[--Index] = Digit - 10 + L'A';
    //     Value /= 16;
    // } while(Value > 0);
    // SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, Buffer + Index);
    // SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, L"\n\r");
    WriteToTerminal(&Terminal, "Image base is at: 0x", 0);
    u32 Index = 32;
    c08 Buffer[32];
    Buffer[--Index] = 0;
    int i = 0;
    if(Index == 30) {
        int i = 1;
    }
    u64 Value = (u64)LoadedImage->ImageBase;
    do {
        u32 Digit = Value % 16;
        if(Digit < 10) Buffer[--Index] = Digit + '0';
        else           Buffer[--Index] = Digit - 10 + 'A';
        Value /= 16;
    } while(Value > 0);
    WriteToTerminal(&Terminal, Buffer + Index, 0);
    WriteToTerminal(&Terminal, "\n", 0);
    Mem_Set(Framebuffer, 0, FramebufferSize);
    DrawTerminal(Framebuffer, &Terminal);
    GOP->Blt(GOP, (efi_graphics_output_blt_pixel*)Framebuffer, EFI_GraphicsOutputBltOperation_BufferToVideo, 0, 0, 0, 0, Renderer.Size.X, Renderer.Size.Y, 0);
    
    b08 Wait = TRUE;
    while(Wait)
        asm ("pause");
    
    
    return EFI_Status_Success;
}