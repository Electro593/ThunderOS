#import "uefi";

wide :: ($str: string) -> *c16 #expand {
   return #insert -> string {
      #import "Basic";
      
      builder: String_Builder;
      init_string_builder(*builder);
      append(*builder, "c16.[");
      
      for 0..str.count-1 {
         append(*builder, "#char \"");
         append(*builder, cast(string) u8.[str.data[it]]);
         append(*builder, "\", ");
      }
      
      append(*builder, "0].data;");
      
      return builder_to_string(*builder);
   }
}

#program_export
uefi_entry :: (image_handle: EFI_Handle, system_table: *EFI_System_Table) -> EFI_Status #c_call {
   result: EFI_Status = ---;
   
   result = efi_call(system_table.console_out.clear_screen, system_table.console_out);
   result = efi_call(system_table.console_out.output_string, system_table.console_out, wide("Hi"));
   
   while true { }
   
   return result;
}