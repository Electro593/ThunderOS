//
// NOTE: This design assumes that invalid memory regions are never marked
// as free. It will happily free an invalid memory region, with undefined
// behavior.
//
// TODO: Reduce metadata allocation thrashing
//
// NOTE: For multi-page allocation, the first block will never be empty
// (the metadata state rules might show it as empty, but it cannot actually
// be allocated)
//

page_allocator := Allocator.{
   page_allocator_proc,
   null
};

page_allocator_proc: Allocator_Proc : (mode: Allocator_Mode, size: s64, old_size: s64, old_memory: *void, allocator_data: *void) -> *void {
   if mode == {
      case .ALLOCATE;
         assert(false, "Page allocate not yet implemented");
         // physical_address, physical_success := allocate_physical_page();
         // if !physical_success return null;
         // virtual_address, virtual_success := allocate_virtual_address(physical_address);
         // if !virtual_success return null;
         // return virtual_address;
      
      case .RESIZE;
         assert(false, "Page resize not yet implemented");
      
      case .FREE;
         assert(false, "Page free not yet implemented");
      
      case .IS_THIS_YOURS;
         return cast(*void) cast(s64) !(old_memory & ~0x01FF_FFFF_FFFF_F000);
      
      case .CAPS;
         return cast(*void) cast(s64) (Allocator_Caps.FREE | .IS_THIS_YOURS);
      
      case;
         assert(false, "The page allocator does not support the % mode", mode);
   }
   
   return null;
}

allocate_physical_page :: () -> address: u64, success: bool {
   lvl3 := *lvl3_data;
   lvl2_index, lvl1_index, lvl0_index: int = ---;
   lvl2, lvl1: *Physical_Allocator_Directory = ---;
   lvl0: *Physical_Allocator_Leaf = ---;
   metadata_count := 0;
   
   lvl3_index, found := find_single_empty_or_partial_child(*lvl3.usage_map);
   if !found return 0, false;
   
   lvl3_entry := lvl3.children[lvl3_index];
   if !(lvl3_entry & .PRESENT) {
      lvl2_address := make_address(lvl3_index, 0, 0, 2);
      
      scratch := make_address(lvl3_index, 0, 0, 3 + metadata_count);
      lvl2_addr, count, success := allocate_virtual_page_during_physical_allocation(lvl2_address, scratch);
      if !success return 0, false;
      lvl2 = xx lvl2_addr;
      metadata_count += count + 1;
      memset(lvl2, 0, PAGE_SIZE);
      
      lvl3.children[lvl3_index] = set_address(lvl3_entry, lvl2_address) | .PRESENT;
      lvl2_index = 0;
   } else {
      lvl2 = xx get_address(lvl3.children[lvl3_index]);
      lvl2_index = find_single_empty_or_partial_child(*lvl2.usage_map);
   }
   
   lvl2_entry := lvl2.children[lvl2_index];
   if !(lvl2.children[lvl2_index] & .PRESENT) {
      lvl1_address := make_address(lvl3_index, lvl2_index, 0, 1);
      
      scratch := make_address(lvl3_index, lvl2_index, 0, 2 + metadata_count);
      lvl1_addr, count, success := allocate_virtual_page_during_physical_allocation(lvl1_address, scratch);
      if !success return 0, false;
      lvl1 = xx lvl1_addr;
      metadata_count += count + 1;
      memset(lvl1, 0, PAGE_SIZE);
      
      lvl2.children[lvl2_index] = set_address(lvl2_entry, lvl1_address) | .PRESENT;
      lvl1_index = 0;
   } else {
      lvl1 = xx get_address(lvl2.children[lvl2_index]);
      lvl1_index := find_single_empty_or_partial_child(*lvl1.usage_map);
   }
   
   lvl1_entry := lvl1.children[lvl1_index];
   if !(lvl1.children[lvl1_index] & .PRESENT) {
      lvl0_address := make_address(lvl3_index, lvl2_index, lvl1_index, 0);
      
      scratch := make_address(lvl3_index, lvl2_index, lvl1_index, 1 + metadata_count);
      lvl0_addr, count, success := allocate_virtual_page_during_physical_allocation(lvl0_address, scratch);
      if !success return 0, false;
      lvl0 = xx lvl0_addr;
      metadata_count += count + 1;
      memset(lvl0, 0, PAGE_SIZE);
      
      lvl1.children[lvl1_index] = set_address(lvl1_entry, lvl0_address >> 8) | .PRESENT;
      lvl0_index = metadata_count;
   } else {
      lvl0 = xx get_address(lvl1.children[lvl1_index]);
      lvl0_index := find_single_empty_or_partial_child(*lvl0.usage_map);
   }
   
   page_address := make_address(lvl3_index, lvl2_index, lvl1_index, lvl0_index + metadata_count);
   
   lvl0_state: Physical_Block_State = ---;
   if metadata_count {
      lvl0_state = mark_single_child_with_metadata(lvl0, metadata_count, .FILLED);
   } else {
      lvl0_state = mark_single_child(lvl0, lvl0_index, .FILLED);
   }
   
   lvl1_state := mark_single_child(lvl1, lvl1_index, lvl0_state);
   lvl2_state := mark_single_child(lvl2, lvl2_index, lvl1_state);
   mark_single_child(lvl3, lvl3_index, lvl2_state);
   
   return page_address, true;
}

free_physical_page_range_for_initialization :: (start: u64, count: int) {
   
}

#scope_file

lvl3_data: Physical_Allocator_Directory;

Physical_Block_State :: enum u8 {
   // The block has no allocated data
   EMPTY :: 0b00;
   
   // The block is entirely allocated
   FILLED :: 0b01;
   
   // Only some sub-blocks, but not all, are allocated
   PARTIAL :: 0b10;
   
   //
   // The block represents physical allocator data
   //
   // When traversing upward, METADATA mirrors its neighbor. This
   // allows metadata pages to be made nonpresent when their blocks
   // are EMPTY or FILLED.
   //
   // When traversing downward, METADATA counts as FILLED, since the
   // pages are in use.
   //
   METADATA :: 0b11;
   
   
   
   // Used to test if a block is either EMPTY or PARTIAL
   IS_FILLED :: 1 << 0;
}

Physical_Allocator_Entry :: enum_flags u64 {
   // Whether the page is resident in memory.
   PRESENT :: 1 << 1;
   
   // Bits [56:12] of the virtual address
   ADDRESS_MASK :: 0x0000_FFFF_FFFF_FFF8;
}

//
// A block state bitmap, allowing for fast parsing of the table.
// Each state in each level corresponds to two states in the level below.
//
// Lvl0 represents Physical_Block_State.IS_FILLED
//
// lvlE has the following format:
//
//    7       4  3     2  1         0
//   [ lvlN-1 ] [ lvlN ] [ reserved ]
//
// The semantics for state merging are as follows:
//
//  EMPTY + EMPTY    -> EMPTY
//  EMPTY + FILLED   -> PARTIAL
//  EMPTY + PARTIAL  -> PARTIAL
//  EMPTY + METADATA -> EMPTY
//
//  FILLED + EMPTY    -> PARTIAL
//  FILLED + FILLED   -> FILLED
//  FILLED + PARTIAL  -> PARTIAL
//  FILLED + METADATA -> FILLED
//
//  PARTIAL + EMPTY    -> PARTIAL
//  PARTIAL + FILLED   -> PARTIAL
//  PARTIAL + PARTIAL  -> PARTIAL
//  PARTIAL + METADATA -> PARTIAL
//
//  METADATA + EMPTY    -> EMPTY
//  METADATA + FILLED   -> FILLED
//  METADATA + PARTIAL  -> PARTIAL
//  METADATA + METADATA -> METADATA
//
Physical_Allocator_Usage_Map :: struct(LAYER_COUNT: int) {
   #insert -> string {
      #import "Basic";
      
      #assert LAYER_COUNT > 0;
      
      builder: String_Builder;
      init_string_builder(*builder);
      defer reset(*builder);
      
      append(*builder, "lvlE: Packed_Array(Physical_Block_State, 2 + 1 + 1, 2);\n");
      for 2..LAYER_COUNT-1 print_to_builder(*builder, "lvl%: Packed_Array(Physical_Block_State, %, 2);\n", LAYER_COUNT-it, 1 << it);
      print_to_builder(*builder, "lvl0: Packed_Array(Physical_Block_State, %, 1);\n", 1 << LAYER_COUNT);
      
      print_to_builder(*builder, "#place lvlE;\n");
      print_to_builder(*builder, "map: Packed_Array(Physical_Block_State, %, 2) = ---;\n", 1 << LAYER_COUNT);
      
      return builder_to_string(*builder);
   }
}

//
// The physical allocator metadata tables.
// Each directory maps 512 sub-tables, leaves map 8192 pages.
//
//   51..43  42..34  33..25  24..12  11..0
//  [PAD 3] [PAD 2] [PAD 1] [ PAL ] [page]
//   4PiB    8TiB    16GiB   32MiB   4KiB
//

Physical_Allocator_Directory :: struct {
   using usage_map: Physical_Allocator_Usage_Map(9) #align 4096;
   
   children: Packed_Array(Physical_Allocator_Entry, 512, 48);
   
   _unused: [832] u8;
}
#assert size_of(Physical_Allocator_Directory) == PAGE_SIZE;

Physical_Allocator_Leaf :: struct {
   using usage_map: Physical_Allocator_Usage_Map(13) #align 4096;
   
   _unused: [1024] u8;
}
#assert size_of(Physical_Allocator_Leaf) == PAGE_SIZE;

make_address :: inline (lvl3_index: int, lvl2_index: int, lvl1_index: int, lvl0_index: int) -> u64 {
   return (cast(u64) lvl3_index << 43)
        | (cast(u64) lvl2_index << 34)
        | (cast(u64) lvl1_index << 25)
        | (cast(u64) lvl0_index << 12);
}

// Read the address from a physical allocator entry, and
// canonicalize it.
get_address :: inline (entry: Physical_Allocator_Entry) -> *void {
   return xx (((cast(s64) (entry & .ADDRESS_MASK)) << 16) >> 7);
}

set_address :: inline (entry: Physical_Allocator_Entry, address: u64) -> Physical_Allocator_Entry {
   return (entry & ~.ADDRESS_MASK) | (.ADDRESS_MASK & xx (address >> 9));
}

merge_state :: inline (left: Physical_Block_State, right: Physical_Block_State) -> Physical_Block_State {
   //  00 + 00 -> 00
   //  00 + 01 -> 10
   //  00 + 10 -> 10
   //  00 + 11 -> 00
   //  01 + 00 -> 10
   //  01 + 01 -> 01
   //  01 + 10 -> 10
   //  01 + 11 -> 01
   //  10 + 00 -> 10
   //  10 + 01 -> 10
   //  10 + 10 -> 10
   //  10 + 11 -> 10
   //  11 + 00 -> 00
   //  11 + 01 -> 01
   //  11 + 10 -> 10
   //  11 + 11 -> 11
   //
   //           m0
   //     ________________
   // 00 | 0 | 0 | 0 | 0 |
   //    |___|___|___|___|
   // 01 | 0 | 1 | 1 | 0 |
   //    |___|___|___|___|
   // 11 | 0 | 1 | 1 | 0 |
   //    |___|___|___|___|
   // 10 | 0 | 0 | 0 | 0 |
   //    |___|___|___|___|
   // l/r 00  01  11  10
   //
   //           m1
   //     ________________
   // 00 | 0 | 1 | 0 | 1 |
   //    |___|___|___|___|
   // 01 | 1 | 0 | 0 | 1 |
   //    |___|___|___|___|
   // 11 | 0 | 0 | 1 | 1 |
   //    |___|___|___|___|
   // 10 | 1 | 1 | 1 | 1 |
   //    |___|___|___|___|
   // l/r 00  01  11  10
   
   l0 := left  >> 0;
   l1 := left  >> 1;
   r0 := right >> 0;
   r1 := right >> 1;
   
   m0 := l0 & r0;
   m1 := ((l1 ^ l0) & ~r0) | ((r1 ^ r0) & ~l0) | (l1 & r1);
   
   return ((m1 & 1) << 1) | (m0 & 1);
}

find_single_empty_or_partial_child :: (using usage_map: *Physical_Allocator_Usage_Map) -> index: int, found: bool {
   if lvlE[1] == .FILLED {
      return -1, false;
   }
   
   index := 2;
   
   for < LAYER_COUNT .. 1 {
      // See if we need the neighbor
      if map[index] & .IS_FILLED {
         index += 1;
      }
      
      index <<= 1;
   }
   
   // Mask off the top bit to index into lvl0
   index &= ~(1 << LAYER_COUNT);
   
   if lvl0[index] & .IS_FILLED {
      index += 1;
   }
   
   return index, true;
}

mark_single_child :: (using directory: *Physical_Allocator_Directory, index: int, state: Physical_Block_State) -> Physical_Block_State {
   // Nothing changed
   if ((xx (children[index] & .PRESENT)) | lvl0[index]) == state {
      return lvlE[1];
   }
   
   lvl0[index] = state;
   children[index] = xx (xx (children[index] & ~.PRESENT)) | (state & 2);
   state = merge_state(state, (xx (children[index ^ 1] & .PRESENT)) | lvl0[index ^ 1]);
   
   index |= 1 << LAYER_COUNT;
   
   for 1 .. LAYER_COUNT-1 {
      index >>= 1;
      map[index] = state;
      state = merge_state(state, map[index ^ 1]);
   }
   
   lvlE[1] = state;
   return state;
}

mark_single_child :: (using leaf: *Physical_Allocator_Leaf, index: int, state: Physical_Block_State) -> Physical_Block_State {
   if lvl0[index] == (state & .IS_FILLED) {
      return lvlE[1];
   }
   
   lvl0[index] = state;
   state = state & lvl0[index ^ 1];
   
   index |= 1 << LAYER_COUNT;
   
   for 1 .. LAYER_COUNT-1 {
      index >>= 1;
      map[index] = state;
      state = merge_state(state, map[index ^ 1]);
   }
   
   lvlE[1] = state;
   return state;
}

mark_single_child_with_metadata :: (using leaf: *Physical_Allocator_Leaf, metadata_count: int, state: Physical_Block_State) -> Physical_Block_State {
   for 0 .. metadata_count-1 {
      lvl0[it] = .METADATA;
   }
   lvl0[metadata_count] = state;
   
   index := 1 << (LAYER_COUNT - 1);
   count := (metadata_count + 1) >> 1;
   odd := metadata_count & 1;
   metadata_count >>= 1;
   
   for index .. index + metadata_count-1 {
      map[it] = .METADATA;
   }
   if odd {
      map[index + metadata_count] = state;
   } else {
      map[index + metadata_count] = merge_state(state, lvl0[(index + metadata_count + 1)]);
   }
   
   for 2 .. LAYER_COUNT-1 {
      index >>= 1;
      count = (count + 1) >> 1;
      
      for index .. index + count-1 {
         map[it] = merge_state(map[(it << 1) + 0], map[(it << 1) + 1]);
      }
   }
   
   state = merge_state(lvlE[2], lvlE[3]);
   lvlE[1] = state;
   return state;
}

#import "Packed_Array";