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

external efi_status EFI_API
EFI_Entry(IN efi_handle ImageHandle,
          IN efi_system_table *SystemTable)
{
    efi_status Status;
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
    Status = SystemTable->BootServices->LocateProtocol(&GOPGUID, NULL, &GOP);
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
    // SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, L"    Pixels per line: ");
    // u32 Index = 32;
    // c16 Buffer[32];
    // Buffer[--Index] = 0;
    // u32 Value = GOP->Mode->Info->PixelsPerScanLine;
    // do {
    //     Buffer[--Index] = (Value % 10) + L'0';
    //     Value /= 10;
    // } while(Value > 0);
    // SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, Buffer + Index);
    // SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, L"\n\r");
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