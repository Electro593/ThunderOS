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

#include <kernel/efi.h>
#include <util/intrin.h>
#include <util/mem.c>
// #include <util/vector.c>
#include <util/str.c>
// #include <render/software.c>
// #include <render/terminal.c>

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
                    Result.Data[--Index] = (UIntValue % 10) + L'0';
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
    SystemTable->BootServices->HandleProtocol(ImageHandle, &EFI_LOADED_IMAGE_PROTOCOL_GUID, (vptr*)&LoadedImage);
    
    SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, L"Image base is at: 0x");
    u32 Index = 32;
    c16 Buffer[32];
    Buffer[--Index] = 0;
    u64 Value = (u64)LoadedImage->ImageBase;
    do {
        u32 Digit = Value % 16;
        if(Digit < 10) Buffer[--Index] = Digit + L'0';
        else           Buffer[--Index] = Digit - 10 + L'A';
        Value /= 16;
    } while(Value > 0);
    SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, Buffer + Index);
    SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, L"\n\r");
    
    SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, L"Waiting for debugger...\n\r");
    
    asm ("int $3");
    #if 0
    
    SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, L"Debugger Connected!\n\r");
    SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, L"Welcome to ThunderOS.\n\r");
    
    Context.PrevContext = (context*)&Context.PrevContext;
    Context.Allocate = Stack_Allocate;
    
    vptr MemBase;
    u64 StackSize = 1 * 1024 * 1024;
    Status = SystemTable->BootServices->AllocatePool(LoadedImage->ImageDataType, StackSize, &MemBase);
    ASSERT(Status == EFI_Status_Success);
    u08 *MemCursor = MemBase;
    
    stack *Stack = Stack_Init(MemCursor, StackSize);
    Context.Stack = Stack;
    MemCursor += StackSize;
    
    
    
    // TODO: Verify that ASSERT works and make a better one
    // TODO: Replace this with GPU stuff eventually
    efi_guid GOPGUID = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    efi_graphics_output_protocol *GOP;
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
    
    efi_graphics_output_blt_pixel Pixel = {0};
    GOP->Blt(GOP, &Pixel, EFI_GraphicsOutputBltOperation_VideoFill, 0, 0, 0, 0, 800, 600, 0);
    
    software_renderer Renderer = {0};
    Renderer.Format = GOP->Mode->Info->PixelFormat == EFI_GraphicsPixelFormat_BlueGreenRedReserved8BitPerColor ? PixelFormat_BGRX_8 : PixelFormat_RGBX_8;
    Renderer.Size = (v2u32){GOP->Mode->Info->HorizontalResolution,
                            GOP->Mode->Info->VerticalResolution};
    Renderer.Framebuffer = (u08*)GOP->Mode->FrameBufferBase;
    Renderer.Pitch = 4 * GOP->Mode->Info->PixelsPerScanLine;
    Renderer.BackgroundColor = V4u08(0, 0, 0, 0);
    Raytrace(&Renderer, (v3r32){0,0,0}, NULL, NULL, NULL, 0);
    
    
    
    // terminal Terminal;
    // Terminal.Pos = (v2u32){20, 20};
    // Terminal.CellCount = (v2u32){20, 5};
    // Terminal.CellSize = (v2u32){20, 40};
    // Terminal.ForegroundColor = (v4u08){255,255,255,127};
    // Terminal.BackgroundColor = (v3u08){0,0,0};
    // Terminal.Text = "Hello, world!\n";
    // DrawTerminal((v3u08*)Renderer.Framebuffer, Renderer.Size, Terminal);
    
    
    
    
    
    
    
    b08 Wait = TRUE;
    while(Wait)
        asm ("pause");
    
    #endif
    
    return EFI_Status_Success;
}