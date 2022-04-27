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

internal void KernelError(c08 *File, u32 Line, c08 *Expression);
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

typedef struct gdt_segment_descriptor {
    u16 Limit;
    u16 BaseP1;
    u08 BaseP2;
    u08 AccessByte;
    u08 Attributes;
    u08 BaseP3;
} gdt_segment_descriptor;

typedef struct gdt {
    gdt_segment_descriptor Entries[8];
} gdt;

typedef enum interrupt_ids {
    Interrupt_Test = 0x00,
} interrupt_ids;

typedef enum idt_attribute_flags {
    IDT_Gate_Interrupt = 0b1110,
    IDT_Gate_Trap      = 0b1111,
    
    IDT_DPL_Ring0 = 0b00,
    IDT_DPL_Ring1 = 0b01,
    IDT_DPL_Ring2 = 0b10,
    IDT_DPL_Ring3 = 0b11,
} idt_attribute_flags;

typedef struct idt_gate_descriptor {
    u16 OffsetP1;
    u16 SegmentSelector;
    u16 Attributes;
    u16 OffsetP2;
    u32 OffsetP3;
    u32 _Reserved1;
} idt_gate_descriptor;

typedef struct idt {
    idt_gate_descriptor Entries[256];
} idt;

extern u08  PortIn08(u16 Address);
extern u16  PortIn16(u16 Address);
extern u32  PortIn32(u16 Address);
extern void PortOut08(u16 Address, u08 Data);
extern void PortOut16(u16 Address, u16 Data);
extern void PortOut32(u16 Address, u32 Data);
extern void WriteGDTR(gdt *GDT, u16 Size);
extern void WriteIDTR(idt *IDT, u16 Size);

typedef enum io_ports {
    Port_PIC1_Command = 0x20,
    Port_PIC1_Data    = 0x21,
    Port_PIC2_Command = 0xA0,
    Port_PIC2_Data    = 0xA1,
    Port_PS2_Data     = 0x60,
    Port_PS2_Status   = 0x64,
    Port_PS2_Command  = 0x64,
} io_ports;
#define PortWait PortOut08(0x80, 0);

#include <kernel/efi.h>
#include <util/intrin.h>
#include <util/mem.c>
#include <util/vector.c>
#include <util/str.c>
#include <drivers/acpi.c>
#include <drivers/ps2.c>
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
    
    switch(TypeIn) {
        case Type_C08p: {
            c08 *C08p = *(c08**)In;
            u32 Length = Mem_BytesUntil(0, C08p);
            
            switch(TypeOut) {
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
    
    if(IsUInt) {
        switch(TypeOut) {
            case Type_Str: {
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

// TODO: Remove dependencies to everything
internal void
KernelError(c08 *File,
            u32 Line,
            c08 *Expression)
{
    if(Context.Framebuffer != NULL) {
        if(Context.Terminal != NULL) {
            Mem_Set(Context.Framebuffer, 0, Context.FramebufferSize);
            
            c08 Buffer[32];
            u32 Index = sizeof(Buffer);
            u32 Value = Line;
            Buffer[--Index] = 0;
            do {
                Buffer[--Index] = (Value % 10) + '0';
                Value /= 10;
            } while(Value > 0);
            
            WriteToTerminal(Context.Terminal, "\n\n", 0);
            WriteToTerminal(Context.Terminal, File, 0);
            WriteToTerminal(Context.Terminal, ":", 0);
            WriteToTerminal(Context.Terminal, Buffer+Index, 0);
            WriteToTerminal(Context.Terminal, ": ERROR: (", 0);
            WriteToTerminal(Context.Terminal, Expression, 0);
            WriteToTerminal(Context.Terminal, ") was FALSE\n", 0);
            DrawTerminal(Context.Framebuffer, Context.Terminal);
        }
    }
    
    while(TRUE)
        asm volatile ("pause");
}

external efi_status
EFI_Entry(u64 LoadBase,
          elf64_dynamic *Dynamics,
          efi_system_table *SystemTable,
          efi_handle ImageHandle)
{
    efi_status Status;
    Mem_Set(&Context, 0, sizeof(context));
    Context.PrevContext = (context*)&Context.PrevContext;
    
    /* handle relocations */
    s32 RelTotalSize = 0;
    s32 RelEntrySize = 0;
    elf64_relocation *Rel = 0;
    for(u32 I = 0; Dynamics[I].Tag != DT_NULL; I++) {
        switch(Dynamics[I].Tag) {
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
    if(Rel && RelEntrySize) {
        while(RelTotalSize > 0) {
            if(ELF64_R_TYPE(Rel->Info) == R_X86_64_RELATIVE) {
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
    Status = SystemTable->BootServices->HandleProtocol(ImageHandle, &EFI_GUID_LOADED_IMAGE_PROTOCOL, (vptr*)&LoadedImage);
    Assert(Status == EFI_Status_Success);
    
    
    
    // idt IDT = {0};
    // WriteIDTR(&IDT, sizeof(idt));
    // IDT.Entries[Interrupt_Test].OffsetP1 = (u16)(u64)InterruptHandler_Test;
    // IDT.Entries[Interrupt_Test].OffsetP2 = (u16)((u64)InterruptHandler_Test >> 16);
    // IDT.Entries[Interrupt_Test].OffsetP3 = (u32)((u64)InterruptHandler_Test >> 32);
    // IDT.Entries[Interrupt_Test].SegmentSelector = 1 << 3;
    
    
    //
    // Memory
    //
    vptr MemBase;
    u64 StackSize = 64 * 1024 * 1024;
    Status = SystemTable->BootServices->AllocatePool(LoadedImage->ImageDataType, StackSize, &MemBase);
    Assert(Status == EFI_Status_Success);
    Context.Stack = Stack_Init(MemBase, StackSize);
    Context.Allocate = Stack_Allocate;
    
    u64 MemoryMapSize, MemoryDescriptorSize;
    Status = SystemTable->BootServices->GetMemoryMap(&MemoryMapSize, NULL, NULL, &MemoryDescriptorSize, NULL);
    efi_memory_descriptor *MemoryMap = Context.Allocate(MemoryMapSize);
    Status = SystemTable->BootServices->GetMemoryMap(&MemoryMapSize, MemoryMap, NULL, NULL, NULL);
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
    // Graphics
    //
    u64 SizeOfGOPInfo;
    efi_graphics_output_protocol *GOP;
    efi_graphics_output_mode_information *Info;
    efi_guid GOPGUID = EFI_GUID_GRAPHICS_OUTPUT_PROTOCOL;
    Status = SystemTable->BootServices->LocateProtocol(&GOPGUID, NULL, (vptr*)&GOP);
    Assert(Status == EFI_Status_Success);
    Status = GOP->QueryMode(GOP, (GOP->Mode == NULL) ? 0 : GOP->Mode->Mode, &SizeOfGOPInfo, &Info);
    Assert(Status == EFI_Status_Success);
    Context.FramebufferSize = GOP->Mode->FrameBufferSize;
    Context.Framebuffer = (u32*)GOP->Mode->FrameBufferBase;
    
    
    //
    // Files
    //
    efi_file_protocol *Volume = NULL;
    efi_simple_file_system_protocol *SFSP = NULL;
    Status = SystemTable->BootServices->HandleProtocol(LoadedImage->DeviceHandle, &EFI_GUID_SIMPLE_FILE_SYSTEM_PROTOCOL, (vptr)&SFSP);
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
    
    
    //
    // ACPI
    //
    acpi_rsdp *RSDP = NULL;
    {
        efi_configuration_table *ConfigTable = SystemTable->ConfigTable;
        for(u32 Index = 0; Index < SystemTable->ConfigTableEntryCount; ++Index)
        {
            if(*(u64*)&ConfigTable->VendorGuid.Data1 == *(u64*)&EFI_GUID_ACPI_TABLE_2_0.Data1 &&
               *(u64*)&ConfigTable->VendorGuid.Data4 == *(u64*)&EFI_GUID_ACPI_TABLE_2_0.Data4) {
                RSDP = ConfigTable->VendorTable;
            }
            
            ++ConfigTable;
        }
    }
    
    acpi ACPI = InitACPI(RSDP);
    // InitInterrupts(ACPI);
    
    
    
    
    // InitPS2Controller(ACPI);
    // DetectPS2Device();
    
    
    
    // u08 Data;
    // while(!PS2_ReceiveData(&Data));
    
    
    // asm ("int $3");
    
    
    Mem_Set(Framebuffer, 0, FramebufferSize);
    WriteToTerminal(&Terminal, "\n----  END  ----", 1);
    DrawTerminal(Framebuffer, &Terminal);
    GOP->Blt(GOP, (efi_graphics_output_blt_pixel*)Framebuffer, EFI_GraphicsOutputBltOperation_BufferToVideo, 0, 0, 0, 0, Renderer.Size.X, Renderer.Size.Y, 0);
    
    
    SystemTable->BootServices->FreePool(Context.Stack);
    
    
    u64 MemoryMapKey;
    Status = SystemTable->BootServices->GetMemoryMap(NULL, NULL, &MemoryMapKey, NULL, NULL);
    SystemTable->BootServices->ExitBootServices(ImageHandle, MemoryMapKey);
    
    
    
    b08 Wait = TRUE;
    while(Wait)
        asm ("pause");
    
    return EFI_Status_Success;
}