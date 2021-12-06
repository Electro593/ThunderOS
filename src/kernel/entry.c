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
#include <kernel/efi.h>

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

internal void Convert(vptr Out, type TypeOut, vptr In, type TypeIn);

#include <util/intrin.h>
#include <util/mem.c>
#include <util/str.c>

internal void
Convert(vptr Out,
        type TypeOut,
        vptr In,
        type TypeIn)
{
    u64 UIntValue = 0;
    
    switch(TypeIn)
    {
        case Type_U08:
            UIntValue = (u64)*(u08*)In;
            goto is_uint;
        case Type_U16:
            UIntValue = (u64)*(u16*)In;
            goto is_uint;
        case Type_U32:
            UIntValue = (u64)*(u32*)In;
            goto is_uint;
        case Type_U64:
            UIntValue = (u64)*(u64*)In;
            
            is_uint:
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
            
            break;
        
        NO_DEFAULT;
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
    SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, L"Debugger Connected!\n\r");
    SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, L"Welcome to ThunderOS.\n\r");
    
    
    
    Context.PrevContext = (context*)&Context.PrevContext;
    Context.Allocate = Stack_Allocate;
    
    vptr MemBase;
    u64 StackSize = 1 * 1024 * 1024;
    Status = SystemTable->BootServices->AllocatePool(EFI_MemoryType_Conventional, StackSize, &MemBase);
    ASSERT(Status == EFI_Status_Success);
    u08 *MemCursor = MemBase;
    
    Context.Stack = Stack_Init(MemCursor, StackSize);
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
    
    u32 Mode = NativeMode; UNUSED(Mode);
    // Status = GOP->SetMode(GOP, Mode);
    ASSERT(Status == EFI_Status_Success);
    SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, L"Entering graphics output mode...\n\r");
    str Output = Printc(L"\tPixels per line: $u32$\n\r", GOP->Mode->Info->PixelsPerScanLine);
    SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, Output.Data);
    
    SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, L"Exiting boot services.\n\r");
    u64 MapKey;
    SystemTable->BootServices->GetMemoryMap(NULL, NULL, &MapKey, NULL, NULL);
    Status = SystemTable->BootServices->ExitBootServices(ImageHandle, MapKey);
    ASSERT(Status == EFI_Status_Success);
    SystemTable->ConsoleIn           = NULL;
    SystemTable->ConsoleInHandle     = NULL;
    SystemTable->ConsoleOut          = NULL;
    SystemTable->ConsoleOutHandle    = NULL;
    SystemTable->StandardErrorHandle = NULL;
    SystemTable->StandardError       = NULL;
    SystemTable->BootServices        = NULL;
    
    return EFI_Status_Success;
}