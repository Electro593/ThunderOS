initialize_gdt :: () {
   code_segment_index := 1;
   data_segment_index := 2;
   tss\_segment_index := 3;
   
   code_segment: Segment_Selector = xx (code_segment_index * size_of(GDT_Descriptor));
   data_segment: Segment_Selector = xx (data_segment_index * size_of(GDT_Descriptor));
   tss\_segment: Segment_Selector = xx (tss\_segment_index * size_of(GDT_Descriptor));
   
   gdt.descriptors[code_segment_index].access_byte = .PRESENT | .READABLE | .CODE | .CODE_OR_DATA;
   gdt.descriptors[code_segment_index].attributes  = .CODE_IS_64_BIT;
   
   gdt.descriptors[data_segment_index].access_byte = .PRESENT | .WRITEABLE | .CODE_OR_DATA;
   gdt.descriptors[data_segment_index].attributes  = 0;
   
   tss_extra_descriptor: *u64 = xx *gdt.descriptors[tss_segment_index+1];
   gdt.descriptors[tss_segment_index].access_byte = .PRESENT | .TSS_64_BIT_AVAILABLE;
   gdt.descriptors[tss_segment_index].attributes  = 0;
   gdt.descriptors[tss_segment_index].limit_p1 = size_of(TSS);
   gdt.descriptors[tss_segment_index].base_p1 = xx ((cast(u64) *tss) >>  0) & 0x0000FFFF;
   gdt.descriptors[tss_segment_index].base_p2 = xx ((cast(u64) *tss) >> 16) & 0x000000FF;
   gdt.descriptors[tss_segment_index].base_p3 = xx ((cast(u64) *tss) >> 24) & 0x000000FF;
   tss_extra_descriptor.*                     = xx ((cast(u64) *tss) >> 32) & 0xFFFFFFFF;
   
   // One more than the index to get the end of the stack
   tss.rsp0 = xx (ring_stacks.data + 1);
   tss.rsp1 = xx (ring_stacks.data + 2);
   tss.rsp2 = xx (ring_stacks.data + 3);
   
   tss.ist1 = xx (ist_stacks.data + 1);
   tss.ist2 = xx (ist_stacks.data + 2);
   tss.ist3 = xx (ist_stacks.data + 3);
   tss.ist4 = xx (ist_stacks.data + 4);
   tss.ist5 = xx (ist_stacks.data + 5);
   tss.ist6 = xx (ist_stacks.data + 6);
   tss.ist7 = xx (ist_stacks.data + 7);
   
   tss.io_map_offset = xx (cast(u64) *tss.io_map - cast(u64) *tss);
   tss.io_map = 0xFFFF;
   
   gdtr: GDTR;
   gdtr_address := *gdtr;
   #asm {
      lgdt [gdtr_address];
   }
   
   _set_segments(code_segment, data_segment, tss_segment);
}

// initialize_idt :: () {
//    idtr: IDTR;
//    idtr_address := *idtr;
//    #asm {
//       lidt [idtr_address];
//    }
// }

#scope_file

gdt: GDT;
// idt: IDT;
tss: TSS;

GDTR :: struct {
   limit: u16 = size_of(GDT) - 1;
   base_address := *gdt #align 2;
}

// IDTR :: struct {
//    limit: u16 = size_of(IDT) - 1;
//    base_address := *idt #align 2;
// }

stack :: #type,distinct [PAGE_SIZE * 1] u8;
ist\_stacks: [7] stack #align 4096;
ring_stacks: [3] stack #align 4096;

GDT :: struct {
   descriptors: [5] GDT_Descriptor;
}

#assert size_of(GDT) <= 0x10000, "The GDT is too large!";
// #assert size_of(IDT) <= 0x1000, "The IDT is too large!";

GDT_Descriptor :: struct {
   limit_p1: u16;
   base_p1: u16;
   base_p2: u8;
   
   access_byte: enum_flags u8 {
      // For code or data
      ACCESSED       :: 1 << 0;
      WRITEABLE      :: 1 << 1; // For data; Otherwise, read-only
      READABLE       :: 1 << 1; // For code; Otherwise, execute-only
      EXPAND_DOWN    :: 1 << 2; // For data
      CONFORMING     :: 1 << 2; // For code
      CODE           :: 1 << 3; // Otherwise, data
      
      // For system
      LDT                   :: 2  << 0;
      TSS_64_BIT_AVAILABLE  :: 9  << 0;
      TSS_64_BIT_BUSY       :: 11 << 0;
      CALL_GATE_64_BIT      :: 12 << 0;
      INTERRUPT_GATE_64_BIT :: 14 << 0;
      TRAP_GATE_64_BIT      :: 15 << 0;
      
      TYPE_MASK      :: 15 << 0;
      
      CODE_OR_DATA   :: 1 << 4; // Otherwise, system
      
      PRIVILEGE_0    :: 0 << 5;
      PRIVILEGE_1    :: 1 << 5;
      PRIVILEGE_2    :: 2 << 5;
      PRIVILEGE_3    :: 3 << 5;
      PRIVILEGE_MASK :: 3 << 5;
      
      PRESENT        :: 1 << 7;
   }
   
   attributes: enum_flags u8 {
      LIMIT_P2_MASK         :: 15 << 0;
      
      CODE_IS_64_BIT        :: 1 << 5; // For code
      
      CODE_IS_32_BIT        :: 1 << 6; // For code; Otherwise, 16-bit
      STACK_IS_ESP          :: 1 << 6; // For SS data; Otherwise, SP
      EXPAND_DOWN_4GB_BOUND :: 1 << 6; // For expand-down data; Otherwise, 64KB 
      
      PAGE_GRANULARITY      :: 1 << 7; // Otherwise, byte-granularity
   }
   
   base_p3: u8;
}

// #placeholder IDT;
// #placeholder interrupt_handlers;

// generate_idt :: () {
//    w := get_current_workspace();
// } @RunWhenReady

IDT_Descriptor :: struct {
   offset_p1: u16;
   
   segment_selector: Segment_Selector;
   
   ist_offset: enum_flags u8 {
      IST_MASK :: 7 << 0;
   }
   
   attributes: enum_flags u8 {
      INTERRUPT_GATE :: 14 << 1;
      TRAP_GATE      :: 15 << 0;
      TYPE_MASK      :: 15 << 0;
      
      PRIVILEGE_0    :: 0 << 5;
      PRIVILEGE_1    :: 1 << 5;
      PRIVILEGE_2    :: 2 << 5;
      PRIVILEGE_3    :: 3 << 5;
      PRIVILEGE_MASK :: 3 << 5;
      
      PRESENT :: 1 << 7;
   }
   
   offset_p2: u16;
   offset_p3: u32;
   
   _reserved: u32;
}

TSS :: struct {
   _reserved0: u32;
   
   rsp0: u64 #align 4;
   rsp1: u64 #align 4;
   rsp2: u64 #align 4;
   
   _reserved1: u64 #align 4;
   
   ist1: u64 #align 4;
   ist2: u64 #align 4;
   ist3: u64 #align 4;
   ist4: u64 #align 4;
   ist5: u64 #align 4;
   ist6: u64 #align 4;
   ist7: u64 #align 4;
   
   _reserved2: u64 #align 4;
   _reserved3: u16;
   
   io_map_offset: u16;
   
   // Not part of the spec, but it's a convenient place to put it
   io_map: u16;
}

Segment_Selector :: enum_flags u16 {
   RPL_MASK  :: 3 << 0;
   
   USING_LDT :: 1 << 2;
   
   INDEX_MASK :: 0xFFF8;
}

kernel_asm :: #library,no_dll "../../build/kernel.o";

_set_segments :: (code_selector: Segment_Selector, stack_selector: Segment_Selector, tss_segment: Segment_Selector) #c_call #no_context #foreign kernel_asm;