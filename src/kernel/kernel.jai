#program_export
_start :: (memory_map: Sparse_Array(EFI_Memory_Descriptor), rsdp: *RSDP) #c_call #no_context {
   ctx: Context = .{
      allocator = .{
         proc = dummy_allocator
      },
      
      logger = dummy_logger,
      
      assertion_failed = dummy_assertion_failed
   };
   
   push_context ctx {
      kernel_entry(memory_map, rsdp);
   }
}

kernel_entry :: (memory_map: Sparse_Array(EFI_Memory_Descriptor), rsdp: *RSDP) {
   #asm {
      cli;
   }
   
   port, has_port := initialize_serial_port(38400);
   if has_port {
      context.logger_data = xx port;
      context.logger = serial_logger;
      
      print_bufferless("Initialized a serial port at I/O address %\n", cast(*void) port);
   }
   
   initialize_page_tables(memory_map);
   
   print_bufferless("Initializing the GDT...\n");
   initialize_gdt();
   
   // TODO: Fix stack address (don't use UEFI's)
   
   print_bufferless("Parsing the ACPI...\n");
   acpi := parse_acpi(rsdp);
   
   print_bufferless("Initializing the APIC...\n");
   initialize_apic(acpi);
   
   print_bufferless("Initializing the IDT...\n");
   initialize_idt();
   
   #asm {
      sti;
   }
   
   // print_bufferless("Initializing the PCI...\n");
   // initialize_pci();
   
   print_bufferless("Halting...\n");
   #asm {
      hlt;
   }
}

kernel_asm :: #library,no_dll "../../build/kernel.o";
get_cr0 :: () -> CR0 #c_call #no_context #foreign kernel_asm;
get_cr3 :: () -> CR3 #c_call #no_context #foreign kernel_asm;
get_cr4 :: () -> CR4 #c_call #no_context #foreign kernel_asm;
set_cr0 :: (cr0: CR0) #c_call #no_context #foreign kernel_asm;
set_cr3 :: (cr3: CR3) #c_call #no_context #foreign kernel_asm;
set_segments :: (code_selector: Segment_Selector, stack_selector: Segment_Selector, tss_segment: Segment_Selector) #c_call #no_context #foreign kernel_asm;
interrupt_switch: u8 #elsewhere kernel_asm;
interrupt_switch_delta: int #elsewhere kernel_asm;

#scope_file

#import "Print";
#import "Sparse_Array";
#import "UEFI";

#load "../shared.jai";
#load "interrupts.jai";

#load "../drivers/acpi.jai";
#load "../drivers/cpu.jai";
#load "../drivers/descriptors.jai";
#load "../drivers/memory.jai";
#load "../drivers/serial.jai";

dummy_allocator :: (mode: Allocator_Mode, size: s64, old_size: s64, old_memory: *void, allocator_data: *void) -> *void {
   context.assertion_failed(#location(#this), "No allocator has been initialized yet!\n");
   return null;
}

dummy_logger :: (message: string, data: *void, info: Log_Info) { }

dummy_assertion_failed :: (location: Source_Code_Location, message: string) {
   context.logger(message, context.logger_data, .{
      location = location,
      common_flags = .ERROR
   });
   
   #asm {
      hlt;
   }
}