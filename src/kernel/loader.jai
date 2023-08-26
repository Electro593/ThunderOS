//
// The bootloader for ThunderOS. UEFI calls _start in kernel/asm.s,
// which then calls uefi_entry. Since this is designed to be a
// learning project first and foremost, all runtime code and types
// will be manually defined.
//

#if CPU != .X64 {
   #assert false, "Unsupported machine type %", CPU;
}

#program_export
uefi_entry :: (image_handle: EFI_Handle, system_table: *EFI_System_Table, image_base: *void) -> EFI_Status #c_call {
   status: EFI_Status = ---;
   
   console_out := system_table.console_out;
   boot_services := system_table.boot_services;
   
   assert :: (expected_status: EFI_Status, message: string) #expand {
      assert(`console_out, `status, expected_status, message);
   }
   
   assert :: (condition: bool, message: string) #expand {
      assert(`console_out, condition, message);
   }
   
   status = efi_call(console_out.clear_screen, console_out);
   
   efi_call(console_out.output_string, console_out, wide("Address of .text: "));
   efi_call(console_out.output_string, console_out, wide(cast(uint) image_base, 16));
   efi_call(console_out.output_string, console_out, wide("\r\n"));
   
   loaded_image: *EFI_Loaded_Image_Protocol;
   status = efi_call(boot_services.handle_protocol, image_handle, *EFI_Loaded_Image_Protocol.GUID, xx *loaded_image);
   assert(.SUCCESS, "Could not get the protocol for this image");
   
   file_system: *EFI_Simple_File_System_Protocol;
   status = efi_call(boot_services.handle_protocol, loaded_image.device_handle, *EFI_Simple_File_System_Protocol.GUID, xx *file_system);
   assert(.SUCCESS, "Could not load the file system");
   
   volume: *EFI_File_Protocol;
   status = efi_call(file_system.open_volume, file_system, *volume);
   assert(.SUCCESS, "Could not open the EFI volume");
   
   kernel_file: *EFI_File_Protocol;
   status = efi_call(volume.open, volume, *kernel_file, wide("\\kernel"), .READ, 0);
   assert(.SUCCESS, "Could not open the kernel executable");
   
   kernel_file_info_size: uint = 0;
   kernel_file_info: *EFI_File_Info = null;
   status = efi_call(kernel_file.get_info, kernel_file, *EFI_File_Info.GUID, *kernel_file_info_size, xx kernel_file_info);
   assert(.BUFFER_TOO_SMALL, "Could not get the size of the kernel's file info");
   
   status = efi_call(boot_services.allocate_pool, .LOADER_DATA, kernel_file_info_size, xx *kernel_file_info);
   assert(.SUCCESS, "Could not allocate space for the kernel's file info");
   status = efi_call(kernel_file.get_info, kernel_file, *EFI_File_Info.GUID, *kernel_file_info_size, xx kernel_file_info);
   assert(.SUCCESS, "Could not get the kernel's file info");
   kernel_file_size := kernel_file_info.file_size;
   status = efi_call(boot_services.free_pool, kernel_file_info);
   assert(.SUCCESS, "Could not free the kernel's file info");
   
   kernel_data: *u8;
   status = efi_call(boot_services.allocate_pool, .LOADER_DATA, kernel_file_size, xx *kernel_data);
   assert(.SUCCESS, "Could not allocate space for the kernel's file");
   status = efi_call(kernel_file.read, kernel_file, *kernel_file_size, kernel_data);
   assert(.SUCCESS, "Could not read the kernel's file");
   
   status = efi_call(kernel_file.close, kernel_file);
   assert(.SUCCESS, "Could not close the kernel's file");
   
   // elf_header: *ELF_Header_64 = xx kernel_data;
   // assert(memcmp(elf_header.magic_number, ELF_Header.MAGIC_NUMBER) == 0, "The kernel has an invalid ELF magic number");
   // assert(elf_header.word_size == .IS_64_BITS, "The kernel's ELF is 32-bit, but we only support 64-bit");
   // assert(elf_header.endianness == .SMALL, "The kernel's ELF is big-endian, but we only support little-endian");
   // assert(elf_header.elf_header_version == 1, "The kernel's ELF header version should be 1");
   // assert(elf_header.os_abi == .SUSTEM_V, "The kernel's ELF header ABI should by System-V");
   // assert(elf_header.type == .DYNAMIC, "The kernel's ELF should be a dynamic library");
   // #if CPU == .X64 {
   //    assert(elf_header.instruction_set == .AMD_X86_64, "The loader is using the AMD x64 instruction set, but the kernel's ELF is not");
   // }
   // assert(elf_header.version == 1, "The kernel's ELF version should be 1");
   
   efi_call(console_out.output_string, console_out, wide("Done!\r\n"));
   
   while true { }
   
   return status;
}



#scope_file



// These are runtime imports, mostly defining types.

#import "elf";
#import "uefi";
#import "wide";

loader_asm :: #library,no_dll "build/loader.o";



// These imports are only for compile-time code, and should not be
// used during runtime.

Basic :: #import "Basic";
Math :: #import "Math";



// We're not linking to the standard library, so we have to replace
// the intrinsics. We're exporting because it prevents name mangling.
#program_export
memset :: (data: *u8, value: u8, count: s32) {
   for 0..count-1 data[it] = value;
}
#program_export
memcpy :: (a: *u8, b: *u8, count: s32) -> s32 {
   i := 0;
   while i < count && a[i] == b[i] i += 1;
   return
      xx ifx i == count then 0
      else ifx a[i] < b[i] then -1
      else 1;
}



// Just a simple assert function to easily verify efi_call outputs
assert :: (console_out: *EFI_Simple_Text_Output_Protocol, given_status: EFI_Status, expected_status: EFI_Status, message: string) #expand {
   if given_status != expected_status {
      efi_call(console_out.output_string, console_out, wide("Error: "));
      efi_call(console_out.output_string, console_out, wide(message));
      efi_call(console_out.output_string, console_out, wide(". We expected "));
      efi_call(console_out.output_string, console_out, wide(expected_status));
      efi_call(console_out.output_string, console_out, wide(", but we got "));
      efi_call(console_out.output_string, console_out, wide(given_status));
      efi_call(console_out.output_string, console_out, wide(".\r\n"));
      while true { };
   }
}

assert :: (console_out: *EFI_Simple_Text_Output_Protocol, condition: bool, message: string) #expand {
   if !condition {
      efi_call(console_out.output_string, console_out, wide("Error: "));
      efi_call(console_out.output_string, console_out, wide(message));
      efi_call(console_out.output_string, console_out, wide(".\r\n"));
      while true { };
   }
}


#if CPU == .X64 {
   // Used for calling UEFI procedures, since they expect the MS ABI,
   // but jai-linux uses the System-V ABI.
   _system_v_x86_64_abi_to_microsoft_x64_abi :: (proc: *void, arg_count: u64, args: ..u64) -> u64 #c_call #no_context #foreign loader_asm;

   efi_call :: (call: (arg1: $A1) -> $R #c_call #no_context, arg1: A1) -> R #c_call #no_context {
      return xx _system_v_x86_64_abi_to_microsoft_x64_abi(xx call, 1, cast(u64) arg1);
   }
   
   efi_call :: (call: (arg1: $A1, arg2: $A2) -> $R #c_call #no_context, arg1: A1, arg2: A2) -> R #c_call #no_context {
      return xx _system_v_x86_64_abi_to_microsoft_x64_abi(xx call, 2, cast(u64) arg1, cast(u64) arg2);
   }
   
   efi_call :: (call: (arg1: $A1, arg2: $A2, arg3: $A3) -> $R #c_call #no_context, arg1: A1, arg2: A2, arg3: A3) -> R #c_call #no_context {
      return xx _system_v_x86_64_abi_to_microsoft_x64_abi(xx call, 3, cast(u64) arg1, cast(u64) arg2, cast(u64) arg3);
   }
   
   efi_call :: (call: (arg1: $A1, arg2: $A2, arg3: $A3, arg4: $A4) -> $R #c_call #no_context, arg1: A1, arg2: A2, arg3: A3, arg4: A4) -> R #c_call #no_context {
      return xx _system_v_x86_64_abi_to_microsoft_x64_abi(xx call, 4, cast(u64) arg1, cast(u64) arg2, cast(u64) arg3, cast(u64) arg4);
   }
   
   efi_call :: (call: (arg1: $A1, arg2: $A2, arg3: $A3, arg4: $A4, arg5: $A5) -> $R #c_call #no_context, arg1: A1, arg2: A2, arg3: A3, arg4: A4, arg5: A5) -> R #c_call #no_context {
      return xx _system_v_x86_64_abi_to_microsoft_x64_abi(xx call, 5, cast(u64) arg1, cast(u64) arg2, cast(u64) arg3, cast(u64) arg4, cast(u64) arg5);
   }
}