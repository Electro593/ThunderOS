/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                         **
**  Author: Aria Seiler                                                    **
**                                                                         **
**  This program is in the public domain. There is no implied warranty,    **
**  so use it at your own risk.                                            **
**                                                                         **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <shared.h>
#include <kernel/efi.h>

#define _EFI_INCLUDE_
#define mode_t u64
#define off_t u64
#define blkcnt_t u64
#define size_t u64
#define int8_t s08
#define int16_t s16
#define int32_t s32
#define int64_t s64
#define uint8_t u08
#define uint16_t u16
#define uint32_t u32
#define uint64_t u64
#define uintn_t u64
#define uintptr_t u64
#define EFI_GRAPHICS_OUTPUT_BLT_OPERATION efi_graphics_output_blt_operation
#define EFI_GRAPHICS_PIXEL_FORMAT efi_graphics_pixel_format
#include <uefi.h>

external s32
main(s32 argc,
     c08 **argv)
{
    efi_handle ImageHandle = IM;
    efi_system_table *SystemTable = (efi_system_table*)ST;
    
    efi_loaded_image_protocol *LoadedImage = NULL;
    SystemTable->BootServices->HandleProtocol(ImageHandle, &EFI_LOADED_IMAGE_PROTOCOL_GUID, (vptr*)&LoadedImage);
    
    printf("Image base is at: 0x%lx\n", LoadedImage->ImageBase);
    printf("Waiting for debugger...\n");
    
    b08 WaitForDebugger = TRUE;
    while(WaitForDebugger)
        asm volatile ("pause");
    
    printf("Debugger connected!\n");
    
    return 0;
}

// #define GNU_EFI_USE_MS_ABI
// #include <../../GNUEFI/inc/efi.h>
// #include <../../GNUEFI/inc/efilib.h>

// EFI_STATUS EFIAPI
// efi_main(EFI_HANDLE ImageHandle,
//          EFI_SYSTEM_TABLE *SystemTable)
// {
//   InitializeLib(ImageHandle, SystemTable);
//   Print(L"Hello, world!\n");
//   return EFI_SUCCESS;
// }

// #include <shared.h>
// #include <kernel/efi.h>

// typedef struct elf64_dyn {
//     s64 Type;
//     union {
//         u64 Value;
//         u64 Pointer;
//     };
// } elf64_dyn;

// typedef struct elf64_rel {
//     u64 OffsetPointer;
//     u64 TypeAndSymbolIndex;
// } elf64_rel;

// #define DT_NULL    0 /* Marks end of dynamic section */
// #define DT_RELA    7 /* Address of Rela relocs */
// #define DT_RELASZ  8 /* Total size of Rela relocs */
// #define DT_RELAENT 9 /* Size of one Rela reloc */
// #define ELF64_R_TYPE(I) ((I) & 0xFFFFFFFF)
// #define R_X86_64_RELATIVE 8 /* Adjust by program base */

// external efi_status
// RelocateImage(s32 LoadBase,
//               elf64_dyn *Sections)
// {
//     s32 RelaTotalSize = 0;
//     s32 RelaEntrySize = 0;
//     elf64_rel *Rela = 0;
    
//     for(s32 I = 0;
//         Sections[I].Type != DT_NULL;
//         ++I)
//     {
//         switch (Sections[I].Type)
//         {
//             case DT_RELA: {
//                 Rela = (elf64_rel*)(u64)((u32)Sections[I].Pointer + LoadBase);
//             } break;
            
//             case DT_RELASZ: {
//                 RelaTotalSize = Sections[I].Value;
//             } break;
            
//             case DT_RELAENT: {
//                 RelaEntrySize = Sections[I].Value;
//             } break;
//         }
//     }
    
//     if(!Rela && RelaEntrySize == 0)
//         return EFI_Status_Success;
    
//     if(!Rela || RelaEntrySize == 0)
//         return EFI_Status_LoadError;
    
//     while(RelaTotalSize > 0)
//     {
//         // Apply the relocations
//         switch(ELF64_R_TYPE(Rela->TypeAndSymbolIndex))
//         {
//             case R_X86_64_RELATIVE: {
//                 u32 *Address = (u32*)(LoadBase + Rela->OffsetPointer);
//                 *Address += LoadBase;
//             } break;
//         }
        
//         Rela = (elf64_rel*)((u08*)Rela + RelaEntrySize);
//         RelaTotalSize -= RelaEntrySize;
//     }
    
//     return EFI_Status_Success;
// }

// external efi_status EFI_API
// EFI_Entry(IN efi_handle ImageHandle,
//           IN efi_system_table *SystemTable)
// {
    // efi_loaded_image_protocol *LoadedImage = NULL;
    // SystemTable->BootServices->HandleProtocol(ImageHandle, &EFI_LOADED_IMAGE_PROTOCOL_GUID, (vptr*)&LoadedImage);
    
    // SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, L"Image Base: ");
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
    
    // SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, L"Entry Point: ");
    // Index = 32;
    // Buffer[32];
    // Buffer[--Index] = 0;
    // Value = (u64)EFI_Entry;
    // do {
    //     u32 Digit = Value % 16;
    //     if(Digit < 10) Buffer[--Index] = Digit + L'0';
    //     else           Buffer[--Index] = Digit - 10 + L'A';
    //     Value /= 16;
    // } while(Value > 0);
    // SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, Buffer + Index);
    // SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, L"\n\r");
    
    
    
    // b08 WaitForDebugger = TRUE;
    // while(WaitForDebugger)
    //     asm volatile ("pause");
    
    // efi_status Status;
    // SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, L"Welcome to ThunderOS.\n\r");
    
    // Context.PrevContext = (context*)&Context.PrevContext;
    // Context.Allocate = Stack_Allocate;
    
    // vptr MemBase;
    // u64 StackSize = 1 * 1024 * 1024;
    // Status = SystemTable->BootServices->AllocatePool(EFI_MemoryType_Conventional, StackSize, &MemBase);
    // ASSERT(Status == EFI_Status_Success);
    // u08 *MemCursor = MemBase;
    
    // Context.Stack = Stack_Init(MemCursor, StackSize);
    // MemCursor += StackSize;
    
    
    // // TODO: Verify that ASSERT works and make a better one
    // // TODO: Replace this with GPU stuff eventually
    // efi_guid GOPGUID = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    // efi_graphics_output_protocol *GOP;
    // Status = SystemTable->BootServices->LocateProtocol(&GOPGUID, NULL, (vptr*)&GOP);
    // ASSERT(Status == EFI_Status_Success);
    
    // efi_graphics_output_mode_information *Info;
    // u32 NumModes, NativeMode;
    // u64 SizeOfInfo;
    // Status = GOP->QueryMode(GOP, (GOP->Mode == NULL) ? 0 : GOP->Mode->Mode, &SizeOfInfo, &Info);
    // ASSERT(Status == EFI_Status_Success);
    // NativeMode = GOP->Mode->Mode;
    // NumModes = GOP->Mode->MaxMode;
    
    // u32 Mode = NativeMode; UNUSED(Mode);
    // // Status = GOP->SetMode(GOP, Mode);
    // ASSERT(Status == EFI_Status_Success);
    // SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, L"Entering graphics output mode...\n\r");
    // str Output = Printc(L"\tPixels per line: $u32$\n\r", GOP->Mode->Info->PixelsPerScanLine);
    // SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, Output.Data);
    
    // SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, L"Exiting boot services.\n\r");
    // u64 MapKey;
    // SystemTable->BootServices->GetMemoryMap(NULL, NULL, &MapKey, NULL, NULL);
    // Status = SystemTable->BootServices->ExitBootServices(ImageHandle, MapKey);
    // ASSERT(Status == EFI_Status_Success);
    // SystemTable->ConsoleIn           = NULL;
    // SystemTable->ConsoleInHandle     = NULL;
    // SystemTable->ConsoleOut          = NULL;
    // SystemTable->ConsoleOutHandle    = NULL;
    // SystemTable->StandardErrorHandle = NULL;
    // SystemTable->StandardError       = NULL;
    // SystemTable->BootServices        = NULL;
    
//     return EFI_Status_Success;
// }