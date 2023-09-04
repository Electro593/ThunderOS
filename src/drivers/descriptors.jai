CODE_SEGMENT_INDEX :: 1;
DATA_SEGMENT_INDEX :: 2;
TSS\_SEGMENT_INDEX :: 3;

CODE_SEGMENT: Segment_Selector : (CODE_SEGMENT_INDEX * size_of(GDT_Descriptor));
DATA_SEGMENT: Segment_Selector : (DATA_SEGMENT_INDEX * size_of(GDT_Descriptor));
TSS\_SEGMENT: Segment_Selector : (TSS\_SEGMENT_INDEX * size_of(GDT_Descriptor));

initialize_gdt :: () {
   gdt.descriptors[CODE_SEGMENT_INDEX].access_byte = .PRESENT | .READABLE | .CODE | .CODE_OR_DATA;
   gdt.descriptors[CODE_SEGMENT_INDEX].attributes  = .CODE_IS_64_BIT;
   
   gdt.descriptors[DATA_SEGMENT_INDEX].access_byte = .PRESENT | .WRITEABLE | .CODE_OR_DATA;
   gdt.descriptors[DATA_SEGMENT_INDEX].attributes  = 0;
   
   tss_extra_descriptor: *u64 = xx *gdt.descriptors[TSS_SEGMENT_INDEX+1];
   gdt.descriptors[TSS_SEGMENT_INDEX].access_byte = .PRESENT | .TSS_64_BIT_AVAILABLE;
   gdt.descriptors[TSS_SEGMENT_INDEX].attributes  = 0;
   gdt.descriptors[TSS_SEGMENT_INDEX].limit_p1 = size_of(TSS);
   gdt.descriptors[TSS_SEGMENT_INDEX].base_p1 = xx ((cast(u64) *tss) >>  0) & 0x0000FFFF;
   gdt.descriptors[TSS_SEGMENT_INDEX].base_p2 = xx ((cast(u64) *tss) >> 16) & 0x000000FF;
   gdt.descriptors[TSS_SEGMENT_INDEX].base_p3 = xx ((cast(u64) *tss) >> 24) & 0x000000FF;
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
   
   gdtr := GDTR.{
      limit = size_of(GDT) - 1,
      base_address = *gdt
   };
   gdtr_address := *gdtr;
   #asm {
      lgdt [gdtr_address];
   }
   
   set_segments(CODE_SEGMENT, DATA_SEGMENT, TSS_SEGMENT);
}

initialize_idt :: () {
   set_idt_entry(divide_error_exception,             0x00, 0, true, 0, CODE_SEGMENT);
   set_idt_entry(debug_exception,                    0x01, 0, true, 0, CODE_SEGMENT);
   set_idt_entry(non_maskable_interrupt,             0x02, 3, true, 0, CODE_SEGMENT);
   set_idt_entry(breakpoint_exception,               0x03, 0, true, 0, CODE_SEGMENT);
   set_idt_entry(overflow_exception,                 0x04, 0, true, 0, CODE_SEGMENT);
   set_idt_entry(bounds_range_exceeded_exception,    0x05, 1, true, 0, CODE_SEGMENT);
   set_idt_entry(invalid_opcode_exception,           0x06, 0, true, 0, CODE_SEGMENT);
   set_idt_entry(device_not_available_exception,     0x07, 0, true, 0, CODE_SEGMENT);
   set_idt_entry(double_fault_exception,             0x08, 7, true, 0, CODE_SEGMENT);
   set_idt_entry(invalid_tss_exception,              0x0A, 0, true, 0, CODE_SEGMENT);
   set_idt_entry(segment_not_present_exception,      0x0B, 0, true, 0, CODE_SEGMENT);
   set_idt_entry(stack_fault_exception,              0x0C, 6, true, 0, CODE_SEGMENT);
   set_idt_entry(general_protection_exception,       0x0D, 2, true, 0, CODE_SEGMENT);
   set_idt_entry(page_fault_exception,               0x0E, 5, true, 0, CODE_SEGMENT);
   set_idt_entry(x87_floating_point_error_exception, 0x10, 0, true, 0, CODE_SEGMENT);
   set_idt_entry(alignment_check_exception,          0x11, 0, true, 0, CODE_SEGMENT);
   set_idt_entry(machine_check_exception,            0x12, 4, true, 0, CODE_SEGMENT);
   set_idt_entry(simd_floating_point_exception,      0x13, 0, true, 0, CODE_SEGMENT);
   set_idt_entry(virtualization_exception,           0x14, 0, true, 0, CODE_SEGMENT);
   set_idt_entry(control_protection_exception,       0x15, 0, true, 0, CODE_SEGMENT);
   set_idt_entry(hypervisor_injection_exception,     0x1C, 0, true, 0, CODE_SEGMENT);
   set_idt_entry(vmm_communication_exception,        0x1D, 0, true, 0, CODE_SEGMENT);
   set_idt_entry(security_exception,                 0x1E, 0, true, 0, CODE_SEGMENT);
   
   interrupt_context = context;
   
   idtr := IDTR.{
      limit = size_of(IDT) - 1,
      base_address = *idt
   };
   idtr_address := *idtr;
   #asm {
      lidt [idtr_address];
   }
}

set_idt_entry :: (handler: Interrupt_Handler, interrupt_index: int, stack_index: int, is_interrupt: bool, privilege_level: int, segment: Segment_Selector) {
   assert(interrupt_index >= 0 && interrupt_index <= 255, "interrupt_index must be 0..255");
   assert(stack_index >= 0 && stack_index <= 7, "stack_index must be 0 (no stack), or 1..7");
   assert(privilege_level >= 0 && privilege_level <= 3, "privilege_level must be 0..3");
   
   address: u64 = xx (*interrupt_switch + interrupt_index * interrupt_switch_delta);
   type: IDT_Descriptor_Attributes = ifx is_interrupt then .INTERRUPT_GATE else .TRAP_GATE;
   dpt: IDT_Descriptor_Attributes = xx (privilege_level << 5);
   
   interrupt_handlers[interrupt_index] = handler;
   
   idt.descriptors[interrupt_index] = .{
      offset_p1 = xx ((address >>  0) & 0x0000FFFF),
      offset_p2 = xx ((address >> 16) & 0x0000FFFF),
      offset_p3 = xx ((address >> 32) & 0xFFFFFFFF),
      
      ist_offset = xx stack_index,
      segment_selector = segment,
      attributes = .PRESENT | dpt | type
   };
}

#program_export
handle_interrupt :: (interrupt_index: u8, error_code: u32) #c_call #no_context {
   push_context interrupt_context {
      interrupt_handlers[interrupt_index](error_code);
      
      isr_index := interrupt_index >> 5;
      bit_index := interrupt_index & 31;
      
      isr := local_apic.in_service[isr_index * 4];
      
      if isr & (1 << bit_index) {
         local_apic.eoi = 0;
      }
   }
}

Interrupt_Handler :: #type (error_code: u32);

Segment_Selector :: enum_flags u16 {
   RPL_MASK  :: 3 << 0;
   USING_LDT :: 1 << 2;
   INDEX_MASK :: 0xFFF8;
}

#scope_file

gdt: GDT;
idt: IDT;
tss: TSS;

//TODO: Stack Guards

stack :: #type,distinct [1] Page;
ist\_stacks: [7] stack #align 4096;
ring_stacks: [3] stack #align 4096;

interrupt_handlers: [256] Interrupt_Handler;

interrupt_context: Context;

GDTR :: struct {
   limit: u16;
   base_address: *GDT #align 2;
}

IDTR :: struct {
   limit: u16;
   base_address: *IDT #align 2;
}

GDT :: struct {
   descriptors: [5] GDT_Descriptor;
}

IDT :: struct {
   descriptors: [256] IDT_Descriptor;
}

#assert size_of(GDT) <= 0x10000, "The GDT is too large!";
#assert size_of(IDT) <= 0x10000, "The IDT is too large!";

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

IDT_Descriptor :: struct {
   offset_p1: u16;
   
   segment_selector: Segment_Selector;
   
   ist_offset: enum_flags u8 {
      IST_MASK :: 7 << 0;
   }
   
   attributes: IDT_Descriptor_Attributes;
   
   offset_p2: u16;
   offset_p3: u32;
   
   _reserved: u32;
}

IDT_Descriptor_Attributes :: enum_flags u8 {
   INTERRUPT_GATE :: 14 << 0;
   TRAP_GATE      :: 15 << 0;
   TYPE_MASK      :: 15 << 0;
   
   PRIVILEGE_0    :: 0 << 5;
   PRIVILEGE_1    :: 1 << 5;
   PRIVILEGE_2    :: 2 << 5;
   PRIVILEGE_3    :: 3 << 5;
   PRIVILEGE_MASK :: 3 << 5;
   
   PRESENT :: 1 << 7;
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