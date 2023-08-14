#import "uefi";

#program_export
uefi_entry :: (load_base: u64, reserved: *void, image_handle: EFI_Handle, system_table: *EFI_System_Table) -> EFI_Status #c_call {
   system_table.console_out.clear_screen(system_table.console_out);
   
   while true { }
   
   return .SUCCESS;
}