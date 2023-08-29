#program_export
_start :: (rsdp: *RSDP) #c_call #no_context {
   ctx: Context = .{
      allocator = .{
         proc = dummy_allocator
      },
      
      logger = dummy_logger,
      
      assertion_failed = dummy_assertion_failed
   };
   
   push_context ctx {
      kernel_entry(rsdp);
   }
}

kernel_entry :: (rsdp: *RSDP) {
   #asm {
      cli;
   }
   
   port, has_port := initialize_serial_port(38400);
   if has_port {
      context.logger_data = xx port;
      context.logger = serial_logger;
      
      print_bufferless("Initialized a serial port at I/O address %\n", cast(*void) port);
   }
   
   print_bufferless("Initializing the GDT...\n");
   initialize_gdt();
   
   // TODO: Fix stack address (don't use UEFI's)
   
   print_bufferless("Parsing the ACPI...\n");
   acpi := parse_acpi(rsdp);
   
   print_bufferless("Initializing the APIC...\n");
   initialize_apic(acpi);
   
   // print_bufferless("Initializing the IDT...\n");
   // initialize_idt();
   
   // print_bufferless("Initializing the PCI...\n");
   // initialize_pci();
   
   #asm {
      sti;
   }
   
   print_bufferless("Halting...\n");
   #asm {
      hlt;
   }
}




#scope_file

#import "Print";
#import "UEFI";

#load "../shared.jai";
#load "../drivers/acpi.jai";
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