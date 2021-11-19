/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                         **
**  Author: Aria Seiler                                                    **
**                                                                         **
**  This program is in the public domain. There is no implied warranty,    **
**  so use it at your own risk.                                            **
**                                                                         **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


#include <shared.h>
#include <amd64/bootloader/efi.h>

external efi_status EFI_API
EFI_Entry(IN efi_handle ImageHandle,
          IN efi_system_table *SystemTable)
{
    SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, L"Hello, testing.");
    
    return EFI_Status_Success;
}