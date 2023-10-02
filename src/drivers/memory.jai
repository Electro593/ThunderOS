initialize_page_tables :: (memory_map: Sparse_Array(EFI_Memory_Descriptor)) {
   set_cr0(get_cr0() & ~.WRITE_PROTECT);
   
   // Set up a recursive mapping
   top_level_page_table.entries[511] = .PRESENT | .WRITABLE | .WRITE_THROUGH | .CACHE_DISABLE | (.ADDRESS_MASK & xx *top_level_page_table);
   
   print_bufferless("\n--------- Memory Map ---------\n");
   print_bufferless("IDX\tPHYS\tVIRT\tCOUNT\tFLAGS\tTYPE\n");
   unusable, external, freeable, kernel: u64;
   for memory_map {
      print_bufferless("[%]:\t%b16\t%b16\t%b16\t%b16\t%\n", it_index, it.physical_start, it.virtual_start, it.number_of_pages, cast(u64) it.attributes, it.type);
      
      if it_index == 53 {
         #asm { mov it, it; }
      }
      if it_index == 55 {
         #asm { mov it, it; }
      }
      
      if it.type == {
         // Cannot be used, mark as not present and unusable
         case .RESERVED; #through;
         case .UNUSABLE; #through;
         case .UNACCEPTED; #through; // TODO What is this?
         case;
            unusable += it.number_of_pages;
         
         //  Externally in use, mark as present and unusable
         case .RUNTIME_SERVICES_CODE; #through; // TODO Can we reclaim?
         case .RUNTIME_SERVICES_DATA; #through; // TODO Can we reclaim?
         case .ACPI_RECLAIM; #through; // TODO When do we reclaim?
         case .ACPI_NVS; #through;
         case .MMIO; #through; // TODO Can we reclaim?
         case .MMIO_PORT_SPACE; #through; // TODO Can we reclaim?
         case .PAL_CODE; // TODO Can we reclaim?
            external += it.number_of_pages;
         
         // To be freed, mark as not present and usable
         case .LOADER_CODE; #through;
         case .BOOT_SERVICES_CODE; #through;
         case .BOOT_SERVICES_DATA; #through;
         case .CONVENTIONAL; #through;
         case .PERSISTENT;
            free_physical_page_range_for_initialization(xx it.physical_start, xx it.number_of_pages);
            freeable += it.number_of_pages;
         
         // The kernel's code, mark as present and usable
         case .LOADER_DATA;
            kernel += it.number_of_pages;
      }
   }
   print_bufferless("\nUnusable: %b16", unusable * 0x1000);
   print_bufferless("\nExternal: %b16", external * 0x1000);
   print_bufferless("\nFreeable: %b16", freeable * 0x1000);
   print_bufferless("\nKernel:   %b16", kernel   * 0x1000);
   print_bufferless("\n------------------------------\n\n");
   
   for memory_map {
      flags: Page_Table_Entry;
      
      if it.type == {
         case .RESERVED; #through;
         case .UNUSABLE; #through;
         case .UNACCEPTED; #through;
         case;
            flags = .PERMANENT | .WRITE_THROUGH | .CACHE_DISABLE;
         
         case .RUNTIME_SERVICES_CODE; #through;
         case .RUNTIME_SERVICES_DATA; #through;
         case .ACPI_RECLAIM; #through;
         case .ACPI_NVS; #through;
         case .MMIO; #through;
         case .MMIO_PORT_SPACE; #through;
         case .PAL_CODE;
            flags = .PERMANENT | .WRITE_THROUGH | .CACHE_DISABLE | .PRESENT;
         
         case .LOADER_CODE; #through;
         case .BOOT_SERVICES_CODE; #through;
         case .BOOT_SERVICES_DATA; #through;
         case .CONVENTIONAL; #through;
         case .PERSISTENT;
            continue;
         
         case .LOADER_DATA;
            flags = .WRITABLE | .PRESENT;
      }
      
      start: u64 = xx it.physical_start & xx Page_Table_Entry.ADDRESS_MASK;
      end := start + (it.number_of_pages << 12);
      mark_page_table_range(start, end, flags, page_table = *top_level_page_table, use_recursive_page_table = false);
   }
   
   set_cr3(.WRITE_THROUGH | .CACHE_DISABLE | (.ADDRESS_MASK & xx *top_level_page_table));
   set_cr0(get_cr0() | .WRITE_PROTECT);
}

allocate_virtual_page_during_physical_allocation :: (address: u64, scratch: u64) -> address: *void, scratch_pages_used: int, success: bool {
   has_lvl5 := has_level_5();
   scratch_pages_used := 0;
   
   lvl5, lvl4, lvl3, lvl2, lvl1: *Page_Table = ---;
   
   lvl4_index: int = xx (ifx has_lvl5 then (address >> 48) & 0x1FF else 0x1FF);
   lvl3_index: int = xx ((address >> 39) & 0x1FF);
   lvl2_index: int = xx ((address >> 30) & 0x1FF);
   lvl1_index: int = xx ((address >> 21) & 0x1FF);
   page_index: int = xx ((address >> 12) & 0x1FF);
   
   lvl4_entry := get_page_table_entry(lvl4_index);
   lvl3_entry := get_page_table_entry(lvl3_index, lvl4_index);
   lvl2_entry := get_page_table_entry(lvl2_index, lvl3_index, lvl4_index);
   lvl1_entry := get_page_table_entry(lvl1_index, lvl2_index, lvl3_index, lvl4_index);
   page_entry := get_page_table_entry(page_index, lvl1_index, lvl2_index, lvl3_index, lvl4_index);
   
   if has_lvl5 && !(lvl4_entry.* & .PRESENT) {
      scratch_pages_used += 1;
      lvl4_entry.* = TABLE_FLAGS | .PRESENT | (.ADDRESS_MASK & xx (scratch + 3));
      lvl4 = get_page_table(lvl4_index);
   }
   
   if !(lvl3_entry.* & .PRESENT) {
      scratch_pages_used += 1;
      lvl3_entry.* = TABLE_FLAGS | .PRESENT | (.ADDRESS_MASK & xx (scratch + 2));
      lvl3 = get_page_table(lvl3_index, lvl4_index);
   }
   
   if !(lvl2_entry.* & .PRESENT) {
      scratch_pages_used += 1;
      lvl2_entry.* = TABLE_FLAGS | .PRESENT | (.ADDRESS_MASK & xx (scratch + 1));
      lvl2 = get_page_table(lvl2_index, lvl3_index, lvl4_index);
   }
   
   if !(lvl1_entry.* & .PRESENT) {
      scratch_pages_used += 1;
      lvl1_entry.* = TABLE_FLAGS | .PRESENT | (.ADDRESS_MASK & xx (scratch + 0));
      lvl1 = get_page_table(lvl1_index, lvl2_index, lvl3_index, lvl4_index);
   }
   
   if page_entry.* & .PRESENT {
      return null, scratch_pages_used, false;
   }
   
   page_entry.* = .WRITABLE | .PRESENT | (.ADDRESS_MASK & xx address);
   page := canonicalize(address);
   return page, scratch_pages_used, true;
}

mark_page_table_range :: (start: u64, end: u64, flags: Page_Table_Entry, $level := 5, page_table: *Page_Table = xx 0xFFFF_FFFF_FFFF_F000, $use_recursive_page_table := true) {
   SHIFT :: 12 + (level-1) * 9;
   ALIGN :: -1 << SHIFT;
   
   #if level == 5 {
      if !has_level_5() {
         mark_page_table_range(start, end, flags, 4, page_table, use_recursive_page_table);
         return;
      }
   }
   
   start_index := (start >> SHIFT) & 0x1FF;
   
   for start_index .. cast(u64) 511 {
      if start >= end break;
      
      page_table.entries[it] |= flags;
      next_start := (start & ALIGN) + (1 << SHIFT);
      defer start = next_start;
      
      #if level == 1 {
         page_table.entries[it] |= .ADDRESS_MASK & xx start;
      } else if page_table.entries[it] & .ADDRESS_MASK {
         #if use_recursive_page_table {
            child_table: *Page_Table = xx (((cast(u64) page_table) << 9) | (it << 12));
         } else {
            child_table: *Page_Table = xx canonicalize(xx (page_table.entries[it] & .ADDRESS_MASK));
         }
         mark_page_table_range(start, end, flags, level-1, child_table, use_recursive_page_table);
      } else {
         #if level == 2 || level == 3 {
            start_is_flat := !(start & ~ALIGN);
            spans_entire_range := next_start <= end;
            
            if start_is_flat && spans_entire_range {
               page_table.entries[it] |= .LARGE_PAGE | xx start;
               continue;
            }
         }
         
         child_table: *Page_Table = xx allocate_physical_page();
         memset(child_table, 0, size_of(Page_Table));
         page_table.entries[it] |= TABLE_FLAGS | (.ADDRESS_MASK & xx child_table);
         mark_page_table_range(start, end, flags, level-1, child_table, use_recursive_page_table);
      }
   }
}

canonicalize :: (address: u64) -> *void {
   shift := get_address_shift();
   return xx (cast(s64) address << shift >> shift);
}

decanonicalize :: (address: *void) -> u64 {
   return xx (cast(u64) address & get_address_mask());
}

#scope_file

physical_allocator: Physical_Allocator_Node;
top_level_page_table: Page_Table;

TABLE_FLAGS: Page_Table_Entry : .WRITE_THROUGH | .CACHE_DISABLE;

Physical_Allocator_Node :: struct {
   next_free: *#this;
}

Page_Table :: struct {
   entries: [512] Page_Table_Entry #align 4096;
}

Page_Table_Entry :: enum_flags u64 {
   PRESENT         :: 1 << 0;
   WRITABLE        :: 1 << 1;
   USER_ACCESSIBLE :: 1 << 2;
   WRITE_THROUGH   :: 1 << 3;
   CACHE_DISABLE   :: 1 << 4;
   ACCESSED        :: 1 << 5;
   DIRTY           :: 1 << 6; // If LARGE_PAGE or Lvl1
   //              :: 1 << 6; // Otherwise
   LARGE_PAGE      :: 1 << 7; // If Lvl3 or Lvl2
   PAT_LVL1        :: 1 << 7; // If Lvl1
   GLOBAL          :: 1 << 8; // If (LARGE_PAGE or Lvl1) and CR4.PAGE_GLOBAL_ENABLE
   //              :: 1 << 8; // Otherwise
   PERMANENT       :: 1 << 9; // (custom)
   //              :: 1 << 10;
   RESTART         :: 1 << 11; // For HLAT paging
   PAT             :: 1 << 12; // If LARGE_PAGE
   LARGE_ADDRESS_MASK_LVL3 :: 0x000F_FFFF_C000_0000; // If LARGE_PAGE and Lvl3
   LARGE_ADDRESS_MASK_LVL2 :: 0x000F_FFFF_FFE0_0000; // If LARGE_PAGE and Lvl2
   ADDRESS_MASK            :: 0x000F_FFFF_FFFF_F000; // Otherwise
   //              :: 1 << 52;
   //              :: 1 << 53;
   //              :: 1 << 54;
   //              :: 1 << 55;
   //              :: 1 << 56;
   //              :: 1 << 57;
   //              :: 1 << 58;
   PROTECTION_KEY_MASK :: 0xF << 59; // If (LARGE_PAGE or Lvl1) and (CR4.USER_PROTECTION_KEYS or CR4.SUPERVISOR_PROTECTION_KEYS)
   //              :: 1 << 59; // Otherwise
   //              :: 1 << 60; // Otherwise
   //              :: 1 << 61; // Otherwise
   //              :: 1 << 62; // Otherwise
   NOT_EXECUTABLE  :: 1 << 63; // If IA32_EFER.NXE
}

has_level_5 :: inline () -> bool {
   return !!(get_cr4() & .LINEAR_ADDRESS_57);
}

get_page_table_entry :: inline (page: int, lvl1 := 511, lvl2 := 511, lvl3 := 511, lvl4 := 511) -> *Page_Table_Entry {
   return xx ((0x7F << 57) | (0x1FF << 48) | (lvl4 << 39) | (lvl3 << 30) | (lvl2 << 21) | (lvl1 << 12) | (page << 3));
}

get_page_table :: inline (lvl1 := 511, lvl2 := 511, lvl3 := 511, lvl4 := 511) -> *Page_Table {
   return xx get_page_table_entry(0, lvl1, lvl2, lvl3, lvl4);
}

get_address_shift :: inline () -> int {
   return ifx has_level_5() then 7 else 14;
}

get_address_mask :: inline () -> u64 {
   return xx ifx has_level_5()
      then 0x01FF_FFFF_FFFF_F000
      else 0x0000_FFFF_FFFF_F000;
}

#import "UEFI";