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
   console_out := system_table.console_out;
   boot_services := system_table.boot_services;
   
   status := efi_call(console_out.clear_screen, console_out);
   
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
   
   kernel_start: () #c_call #no_context = xx load_elf(boot_services, console_out, kernel_data);
   kernel_start();
   
   efi_call(console_out.output_string, console_out, wide("Done!\r\n"));
   
   while true { }
   
   return status;
}



#scope_file



// These are runtime imports, mostly defining types.

#import "ELF";
#import "Shared";
#import "Sparse_Array";
#import "UEFI";
#import "Wide";

loader_asm :: #library,no_dll "build/loader.o";



// These imports are only for compile-time code, and should not be
// used during runtime.

Basic :: #import "Basic";
Math :: #import "Math";



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

assert :: (condition: bool, message: string) #expand {
   assert(`console_out, condition, message);
}

assert :: (expected_status: EFI_Status, message: string) #expand {
   assert(`console_out, `status, expected_status, message);
}

load_elf :: (boot_services: *EFI_Boot_Services, console_out: *EFI_Simple_Text_Output_Protocol, data: *u8) -> (() #no_call #no_context) #c_call #no_context {
   elf: ELF;
   
   using elf.header;
   elf.header = xx data;
   elf.data = data;
   
   elf.program_headers = .{
      data = data + program_header_offset,
      entry_size = program_header_entry_size,
      count = program_header_entry_count
   };
   
   elf.section_headers = .{
      data = data + section_header_offset,
      entry_size = section_header_entry_size,
      count = section_header_entry_count
   };
   
   shstrtab := elf.section_headers[string_table_index];
   elf.section_header_string_table.count = xx shstrtab.size_in_file;
   elf.section_header_string_table.data = data + shstrtab.offset_in_file;
   
   assert(memcmp(magic_number.data, ELF_Header.MAGIC_NUMBER.data, 4) == 0, "The kernel has an invalid ELF magic number");
   assert(word_size == .IS_64_BITS, "The ELF is 32-bit, but we only support 64-bit");
   assert(endianness == .LITTLE, "The ELF is big-endian, but we only support little-endian");
   assert(elf_header_version == 1, "The ELF header version should be 1");
   assert(os_abi == .SYSTEM_V, "The ELF header ABI should by System-V");
   #if CPU == .X64 {
      assert(instruction_set == .AMD_X86_64, "The loader is using the AMD x64 instruction set, but the kernel's ELF is not");
   }
   assert(elf_version == 1, "The ELF version should be 1");
   
   if type == {
      case .NONE;
      
      case .RELOCATABLE;
         assert(false, "The RELOCATABLE ELF type is not yet implemented");
      
      case .EXECUTABLE;
         assert(false, "The EXECUTABLE ELF type is not yet implemented");
      
      case .DYNAMIC;
         return load_elf_dynamic(boot_services, console_out, elf);
      
      case .CORE;
         assert(false, "The CORE ELF type is not yet implemented");
      
      case;
         assert(false, "Unknown ELF type");
   }
   
   return null;
}

load_elf_dynamic :: (boot_services: *EFI_Boot_Services, console_out: *EFI_Simple_Text_Output_Protocol, using elf: ELF) -> (() #no_call #no_context) #c_call #no_context {
   base_address: u64 : 0x1000;
   
   for program_header: program_headers {
      using program_header;
      
      if type == {
         case .NULL;
         
         case .LOADABLE;
            min_address := base_address + align_down(virtual_address, PAGE_SIZE);
            max_address := base_address + align_up(virtual_address + size_in_memory, PAGE_SIZE);
            address := base_address + virtual_address;
            page_count: uint = (max_address - min_address) / PAGE_SIZE;
            memory_type: EFI_Memory_Type = ifx flags & .EXECUTABLE then .LOADER_CODE else .LOADER_DATA;
            status := efi_call(boot_services.allocate_pages, .ADDRESS, memory_type, page_count, xx *min_address);
            assert(.SUCCESS, "Could not allocate memory for an ELF segment");;
            
            segment_data := data + offset_in_file;
            efi_call(boot_services.copy_mem, xx address, xx segment_data, size_in_file);
            efi_call(boot_services.set_mem, xx (address + size_in_file), size_in_memory - size_in_file, 0);
         
         case .NOTE;
            notes_start := data + offset_in_file;
            notes_end := notes_start + size_in_file;
            print_notes(console_out, xx notes_start, xx notes_end);
         
         case .PROGRAM_HEADER;
         
         case;
            efi_call(console_out.output_string, console_out, wide("Encountered an unhandled ELF program header type: "));
            efi_call(console_out.output_string, console_out, wide(cast(u32) type, 16));
            efi_call(console_out.output_string, console_out, wide("\r\n"));
      }
   }
   
   for section_header: section_headers {
      using section_header;
      
      if type == {
         case .RELOCATIONS_WITH_ADDENDS;
            relocations: Sparse_Array(ELF_Relocation_With_Addend) = .{
               data = data + offset_in_file,
               entry_size = entry_size,
               count = size_in_file / entry_size
            };
            
            symbols_header := section_headers[linked_section_index];
            symbols: Sparse_Array(ELF_Symbol) = .{
               data = data + symbols_header.offset_in_file,
               entry_size = symbols_header.entry_size,
               count = symbols_header.size_in_file / symbols_header.entry_size
            };
            
            // TODO consecutive records are composed
            
            for relocation: relocations {
               value_address := base_address + relocation.offset;
               symbol := symbols[relocation.symbol_index];
               
               #if CPU == .X64 {
                  if relocation.type == {
                     case .NONE;
                     
                     case .A64;
                        (cast(*u64) value_address).* = symbol.value + xx relocation.addend;
                     
                     case .GLOB_DAT;
                        (cast(*u64) value_address).* = symbol.value;
                     
                     case .JUMP_SLOT;
                        (cast(*u64) value_address).* = ifx symbol.value then symbol.value + base_address else 0;
                     
                     case .RELATIVE;
                        (cast(*u64) value_address).* = base_address + xx relocation.addend;
                     
                     case;
                        efi_call(console_out.output_string, console_out, wide("Encountered an unhandled AMD64 ELF relocation type: "));
                        efi_call(console_out.output_string, console_out, wide(cast(u32) relocation.type));
                        efi_call(console_out.output_string, console_out, wide("\r\n"));
                  }
               }
            }
         
         case;
            efi_call(console_out.output_string, console_out, wide("Encountered an unhandled ELF section header type: "));
            efi_call(console_out.output_string, console_out, wide(cast(u32) type));
            efi_call(console_out.output_string, console_out, wide("\r\n"));
      }
   }
   
   return xx (base_address + header.entry_point_address);
}

print_notes :: (console_out: *EFI_Simple_Text_Output_Protocol, notes: *ELF_Note, notes_end: *ELF_Note) #c_call #no_context {
   note := notes;
   
   while note < notes_end {
      using note;
      
      name: *u8 = cast(*u8) note + size_of(ELF_Note);
      descriptor: *u8 = name + align_up(name_size, 8);
      
      efi_call(console_out.output_string, console_out, wide("Note "));
      if name_size > 0 {
         efi_call(console_out.output_string, console_out, wide("from "));
         buf := u16.[0, 0];
         for 0..name_size-1 {
            buf[0] = name[it];
            efi_call(console_out.output_string, console_out, buf.data);
         }
      }
      efi_call(console_out.output_string, console_out, wide(" (type "));
      efi_call(console_out.output_string, console_out, wide(type));
      efi_call(console_out.output_string, console_out, wide(")"));
      if descriptor_size > 0 {
         efi_call(console_out.output_string, console_out, wide(": "));
         buf := u16.[0, 0];
         for 0..descriptor_size-1 {
            buf[0] = descriptor[it];
            efi_call(console_out.output_string, console_out, buf.data);
         }
      }
      efi_call(console_out.output_string, console_out, wide("\r\n"));
      
      note = xx (descriptor + align_up(descriptor_size, 8));
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
   
   
   efi_call :: (call: (arg1: $A1, arg2: $A2, arg3: $A3) #c_call #no_context, arg1: A1, arg2: A2, arg3: A3) #c_call #no_context {
      _system_v_x86_64_abi_to_microsoft_x64_abi(xx call, 3, cast(u64) arg1, cast(u64) arg2, cast(u64) arg3);
   }
}