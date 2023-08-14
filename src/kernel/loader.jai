#import "uefi";

start :: () {
   image_handle: EFI_Handle = ---;
   system_table: *EFI_System_Table = ---;
   
   #asm {
      image_handle === c;
      system_table === d;
   }
   
   uefi_entry(image_handle, system_table);
}

uefi_entry :: (image_handle: EFI_Handle, system_table: *EFI_System_Table) -> EFI_Status {
   system_table.console_out.output_string(system_table.console_out, c16.[#char "H", #char "i", #char "\r", #char "\n", 0].data);
   
   return .SUCCESS;
}