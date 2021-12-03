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
#include <util/mem.c>
#include <util/str.c>

external efi_status EFI_API
EFI_Entry(IN efi_handle ImageHandle,
          IN efi_system_table *SystemTable)
{
    efi_status Status;
    SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, L"Welcome to ThunderOS.");
    
    
    vptr MemBase;
    
    u08 *MemCursor = MemBase;
    u64 StackSize = 1 * 1024 * 1024;
    Context.Stack = Stack_Init(MemCursor;
    MemCursor += StackSize;
    Context.Stack->Size = StackSize - sizeof(stack);
    Stack_Push();
    Callbacks.TempAllocate = Stack_Allocate;
    
    
    
    // TODO: Replace this with GPU stuff eventually
    efi_guid GOPGUID = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    efi_graphics_output_protocol *GOP;
    Status = SystemTable->BootServices->LocateProtocol(&GOPGUID, NULL, &GOP);
    ASSERT(Status == EFI_Status_Success);
    
    efi_graphics_output_mode_information *Info;
    u64 SizeOfInfo, NumModes, NativeMode;
    Status = GOP->QueryMode(GOP, (GOP->Mode == NULL) ? NULL : GOP->Mode->Mode, &SizeOfInfo, &Info);
    ASSERT(Status == EFI_Status_Success);
    NativeMode = GOP->Mode->Mode;
    NumModes = GOP->Mode->MaxMode;
    
    u32 Mode = NativeMode;
    Status = GOP->SetMode(GOP, Mode);
    ASSERT(Status == EFI_Status_Success);
    SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, L"Graphics output mode\n");
    SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, L"\tPixels per line: ");
    u32 Index = 32;
    c16 Buffer[32];
    Buffer[--Index] = 0;
    u32 Value = GOP->Mode->Info->PixelsPerScanLine;
    do {
        Buffer[--Index] = (Value % 10) + L"0";
        Value /= 10;
    } while(Value > 0);
    SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, Buffer);
    SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, L"\n");
    
    
    
    SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, L"Exiting boot services.");
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