//////////////////////////////////////////////////////////////////////////////////////////
//
//                                  Physical Allocator
//
//----------------------------------------------------------------------------------------
//
// TODO: Prevent thrashing of alloc/dealloc metadata pages?
//
//
//
//
//
//

#scope_file

#load "../util/packed_array.jai";






//
// A pointer to a virtual page in the physical allocator. Its format is as follows:
//
// 4-level page tables:
//     60    36  35          0
//    [ flags ] [ page index ]
//
// 5-level page tables:
//     60    45  44          0
//    [ flags ] [ page index ]
//
Virtual_Page_Pointer_Flags :: enum_flags {
   PRESENT;
}
Virtual_Page_Pointer :: struct {
   val : u64;
}

get_shift_and_mask :: inline () -> shift: int, mask: u64 {
   if cr4_has_flags(.LA57) return 45, 0x01FF_FFFF_FFFF_F000;
   return 36, 0x0000_FFFF_FFFF_F000;
}

operator & :: inline (using a: Virtual_Page_Pointer, b: Virtual_Page_Pointer_Flags) -> Virtual_Page_Pointer_Flags #symmetric {
   shift := get_shift_and_mask();
   return (xx val >> shift) & flags;
}

operator | :: inline (using a: Virtual_Page_Pointer, b: Virtual_Page_Pointer_Flags) -> Virtual_Page_Pointer_Flags #symmetric {
   shift := get_shift_and_mask();
   return (xx val >> shift) | flags;
}

virtual_page_pointer :: inline (address: u64, flags: Virtual_Page_Pointer_Flags) -> Virtual_Page_Pointer {
   shift, mask := get_shift_and_mask();
   return .{(flags << shift) | (address >> 12) & mask};
}

// Returns a canonical address
get_ptr :: inline (using ptr: Virtual_Page_Pointer) -> *void {
   shift := get_shift_and_mask();
   return xx (cast(s64) val << (64-shift) >> (64-shift));
}




//
// Each state in each level corresponds to two states in the level below.
//
// lvlE has the following format:
//
//    7       4  3     2  1         0
//   [ lvlN-1 ] [ lvlN ] [ reserved ]
//

Physical_Block_State :: enum {
   // Bit 0 is whether a block has data in it
   // Bit 1 is the block's expected .PRESENT flag
   // (e.g. EMPTY and FILLED metadata pages can be freed)
   EMPTY    :: 0b00;
   FILLED   :: 0b01;
   PARTIAL  :: 0b11;
   
   // .METADATA is a special number for easily freeing metadata
   // pages.
   //   When traversing upward, it simply mirrors its neighbor.
   // So if its neighbor is empty or filled, it'll say the whole
   // chunk is like that, so its page can be deallocated despite
   // metadata using it.
   //   When traversing downward, it just counts as filled, so
   // it's never re-allocated.
   METADATA :: 0b10;
}

Physical_Allocator_Usage_Map :: struct(Count_Exp: int) {
   #insert -> string {
      #import "Basic";
      
      #assert Count_Exp > 0;
      
      builder: String_Builder;
      init_string_builder(*builder);
      defer reset(*builder);
      
      print_to_builder(*builder, "lvlE : Packed_Array(%, Physical_Block_State, %);\n", 4, 2);
      for 2..Count_Exp-1 print_to_builder(*builder, "lvl% : Packed_Array(%, Physical_Block_State, %);\n", Count_Exp-it, 1 << it, 2);
      if Count_Exp > 2 print_to_builder(*builder, "lvl0 : Packed_Array(%, Physical_Block_State, %);\n", 1 << Count_Exp, 1);
      
      print_to_builder(*builder, "#place lvlE;\n");
      print_to_builder(*builder, "map : Packed_Array(%, Physical_Block_State, %) = ---;\n", 1 << Count_Exp, 2);
      
      return builder_to_string(*builder);
   }
}





Physical_Allocator_Leaf :: struct {
   // If the lowest level is set, the page is allocated
   using usage_map : Physical_Allocator_Usage_Map(13);
   
   _unused : [1024] u8;
}
#assert size_of(Physical_Allocator_Leaf) == 4096;

Physical_Allocator_Directory :: struct {
   // If the lowest level is set, the page is partial.
   // Whether it is empty of full depends on the presence
   // of its respective pointer.
   using usage_map : Physical_Allocator_Usage_Map(9);
   
   children : Packed_Array(512, Virtual_Page_Pointer, 61);
}
#assert size_of(Physical_Allocator_Directory) == 4096;

#add_context physical_allocator: *Physical_Allocator_Directory;




find_single_free_child :: (using usage_map: Physical_Allocator_Usage_Map) -> index: u64, found: bool {
   code := map[1];
   i : u64 = 2;
   if code == .FILLED return 0, false;
   
   for 0..Count_Exp-1 {
      code = map[i];
      if code == .FILLED || code == .METADATA {
         i += 1;
         assert(map[i+1] != .FILLED, "This block is marked as filled, but its parent says it isn't");
      } else if code == .EMPTY {
         i += xx (map[i+1] == .PARTIAL);
      }
      i <<= 1;
   }
   
   i &= ~(1 << Count_Exp);
   assert(lvl0[i] != 1, "This block is marked as filled, but its parent says it isn't");
   return i, true;
}

mark_single_child :: (using usage_map: Physical_Allocator_Usage_Map, index: u64, state: Physical_Block_State) -> top: Physical_Block_State, done: bool {
   lvl0[index] = state;
   index |= (1 << Count_Exp);
   code := state;
   
   for 0..Count_Exp-1 {
      buddy := map[index ^ 1];
      index >>= 1;
      prev := map[index];
      if code != buddy && buddy != .METADATA {
         code = ifx code == .METADATA then buddy else .PARTIAL;
      }
      
      // It didn't change, so no need to update the rest
      if code == prev return code, true;
      map[index] = code;
   }
   
   return code, false;
}

initialize_usage_map :: inline (using usage_map: Physical_Allocator_Usage_Map, state: Physical_Block_State, metadata_count := 0) {
   state8 := (state << 6) | (state << 4) | (state << 2) | (state << 0);
   
   //TODO: memset
   bytes : *u8 = *usage_map;
   for 0..size_of(type_of(usage_map)) bytes[it] = state8;
   
   for 0..metadata_count-1 mark_single_child(usage_map, it, .METADATA);
}

get_or_allocate :: (using dir: *Physical_Allocator_Directory, index: u64, metadata_addr: u64, metadata_count: int) -> child: *Physical_Allocator_Leaf, allocated: bool {
   child := dir[index];
   
   if(child & .PRESENT) return get_ptr(child), false;
   
   virtual_addr := allocate_virtual_page(metadata_addr);
   dir[index] = virtual_page_pointer(virtual_addr, child | .PRESENT);
   
   if metadata_count >= 0 {
      new_child : *Physical_Allocator_Leaf = virtual_addr;
      initialize_usage_map(new_child.usage_map, usage_map.lvl0[index], metadata_count+1);
   } else {
      new_child : *Physical_Allocator_Directory = virtual_addr;
      initialize_usage_map(new_child.usage_map, usage_map.lvl0[index], 0);
      
      //TODO: memset
      bytes : *u8 = *new_child.children;
      for 0..size_of(type_of(new_child.children)) bytes[it] = 0;
   }
   
   return virtual_addr, true;
}

get_or_allocate :: inline (using dir: *Physical_Allocator_Directory, index: u64, metadata_addr: u64) -> child: *Physical_Allocator_Directory, allocated: bool {
   child, allocated := get_or_allocate(dir, index, metadata_addr, -1);
   return xx child, allocated;
}

maybe_free :: (using dir: *Physical_Allocator_Directory, index: u64, state: Physical_Block_State) -> freed: bool {
   if state != .EMPTY && state != .FILLED return false;
   
   child := children[index];
   if(!(child & .PRESENT)) return false;
   
   free_virtual_page(get_ptr(child));
   children[index] = .{};
   
   return true;
}

#scope_export

allocate_virtual_page :: (physical_address: u64) -> *void {
   assert(false, "Unimplemented");
}

allocate_physical_page :: () -> address: u64, success: bool {
   found : bool = ---;
   i3, i2, i1, i0 : u64 = ---;
   
   dir3 := context.physical_allocator;
   
   i3, found = find_single_free_child(dir3.usage_map);
   if(!found) return 0, false;
   addr := i3 << 43;
   dir2, alloc2 := get_or_allocate(dir3, i3, addr | 0x2000);
   
   i2, found = find_single_free_child(dir2.usage_map);
   assert(found);
   addr |= i2 << 34;
   dir1, alloc1 := get_or_allocate(dir2, i2, addr | 0x1000);
   
   i1, found = find_single_free_child(dir1.usage_map);
   assert(found);
   addr |= i1 << 25;
   leaf := get_or_allocate(dir1, i1, addr, alloc1 + alloc2);
   
   i0, found = find_single_free_child(leaf.usage_map);
   assert(found);
   addr |= i0 << 12;
   
   state, done := mark_single_child(leaf.usage_map, i0, .FILLED);
   if !done {
      maybe_free(dir1, i1, state);
      state, done = mark_single_child(dir1.usage_map, i1, state);
   }
   if !done {
      maybe_free(dir2, i2, state);
      state, done = mark_single_child(dir2.usage_map, i2, state);
   }
   if !done {
      maybe_free(dir3, i3, state);
      state, done = mark_single_child(dir3.usage_map, i3, state);
   }
   
   return addr, true;
}

free_physical_page :: (address: u64) -> success: bool {
   i3, i2, i1, i0 := address >> 43, address >> 34, address >> 25, address >> 12;
   dir3 := context.physical_allocator;
   dir2, alloc2 := get_or_allocate(dir3, i3, address & 0x000F_F800_0000_0000 | 0x2000);
   dir1, alloc1 := get_or_allocate(dir2, i2, address & 0x000F_FFFC_0000_0000 | 0x1000);
   leaf := get_or_allocate(dir1, i1, address & 0x000F_FFFF_FE00_0000, alloc1 + alloc2);
   
   state, done := mark_single_child(leaf.usage_map, i0, .EMPTY);
   if !done {
      maybe_free(dir1, i1, state);
      state, done = mark_single_child(dir1.usage_map, i1, state);
   }
   if !done {
      maybe_free(dir2, i2, state);
      state, done = mark_single_child(dir2.usage_map, i2, state);
   }
   if !done {
      maybe_free(dir3, i3, state);
      state, done = mark_single_child(dir3.usage_map, i3, state);
   }
   
   return true;
}