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
    SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, L"Welcome to ThunderOS.");
    
    
    
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