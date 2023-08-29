parse_acpi :: (rsdp: *RSDP) -> ACPI {
   acpi: ACPI;
   
   acpi.rsdp = rsdp;
   validate_rsdp(rsdp);
   rsdp_name := get_name(rsdp);
   assert(rsdp_name == "RSD PTR ", "The RSDP table had the name % instead of \"RSD PTR \"", rsdp_name);
   
   acpi.xsdt = xx rsdp.rsdt;
   validate_sdt(acpi.rsdt);
   rsdt_name := get_name(acpi.xsdt);
   assert(rsdt_name == "RSDT", "The RSDT table had the name % instead of \"RSDT\"", rsdt_name);
   
   if rsdp.revision >= .RSDP_2_0 {
      acpi.xsdt = (cast(*RSDP_2_0) rsdp).xsdt;
      validate_sdt(acpi.xsdt);
      xsdt_name := get_name(acpi.xsdt);
      assert(xsdt_name == "XSDT", "The XSDT table had the name % instead of \"XSDT\"", xsdt_name);
   }
   
   handle_sdt :: (acpi: *ACPI, sdt: *SDT) {
      validate_sdt(sdt);
      
      if get_name(sdt) == {
         case "APIC"; acpi.madt = xx sdt;
         case "FACP"; acpi.fadt = xx sdt;
      }
   }
   
   if acpi.xsdt {
      tables: [] *SDT;
      tables.data = xx (acpi.xsdt + 1);
      tables.count = (acpi.xsdt.length - size_of(XSDT)) / size_of(*SDT);
      for tables handle_sdt(*acpi, it);
   } else {
      tables: [] u32;
      tables.data = xx (acpi.rsdt + 1);
      tables.count = (acpi.rsdt.length - size_of(RSDT)) / size_of(*SDT);
      for tables handle_sdt(*acpi, xx it);
   }
   
   return acpi;
}

initialize_apic :: (using acpi: ACPI) {
   if madt.flags & .PCAT_COMPAT {
      // Initialize the PIC
      out8(MASTER_PIC_COMMAND_PORT, xx PIC_ICW1.INITIALIZE | .HAS_ICW4);
      out8(SLAVE\_PIC_COMMAND_PORT, xx PIC_ICW1.INITIALIZE | .HAS_ICW4);
      
      out8(MASTER_PIC_DATA_PORT, 0x20); // Interrupts 32-39
      out8(SLAVE\_PIC_DATA_PORT, 0x28); // Interrupts 40-47
      
      out8(MASTER_PIC_DATA_PORT, 4); // Slave at IRQ 2
      out8(SLAVE\_PIC_DATA_PORT, 2); // Cascade identity
      
      out8(MASTER_PIC_DATA_PORT, xx PIC_ICW4.USE_8086_MODE);
      out8(SLAVE\_PIC_DATA_PORT, xx PIC_ICW4.USE_8086_MODE);
      
      // Mask everything
      out8(MASTER_PIC_DATA_PORT, 0xFF);
      out8(SLAVE\_PIC_DATA_PORT, 0xFF);
   }
   
   local_apic = xx madt.local_apic_address;
   
   entries: *u8 = xx (madt + 1);
   index := 0;
   while entries < cast(*u8) madt + madt.length {
      entry: *MADT_Entry = xx (entries + index);
      
      if entry.type == {
         case .LOCAL_APIC_ADDRESS_OVERRIDE;
            override: *Local_APIC_Override = xx entry;
            local_apic = override.local_apic_address;
      }
      
      entries += entry.length;
   }
   
   local_apic.spurious_interrupt_vector |= .ENABLE_APIC | .SPURIOUS_VECTOR_MASK & xx spurious_vector;
}

spurious_vector :: 0xFF;

local_apic: *Local_APIC;

ACPI :: struct {
   rsdp: *RSDP;
   
   rsdt: *RSDT;
   xsdt: *XSDT;
   
   fadt: *FADT;
   madt: *MADT;
}

RSDP :: struct {
   signature: [8]u8;
   checksum: u8;
   
   oem_id: [6]u8;
   
   revision: enum u8 {
      RSDP_1_0 :: 0;
      RSDP_2_0 :: 2;
   }
   
   rsdt: u32;
} #no_padding

RSDP_1_0 :: RSDP;

RSDP_2_0 :: struct {
   #as using _: RSDP_1_0;
   
   length: u32;
   
   xsdt: *XSDT;
   
   extended_checksum: u8;
   
   _reserved: [3]u8;
} #no_padding

RSDT :: struct {
   #as using _: SDT;
   
   // entries: .. u32;
} #no_padding

XSDT :: struct {
   #as using _: SDT;
   
   // entries: .. *SDT;
} #no_padding

FADT :: struct {
   #as using _: SDT;
   
   firmware_control: u32;
   
   dsdt: u32;
   
   _reserved0: u8;
   
   preferred_power_management_profile: u8;
   
   sci_interrupt:    u16;
   sci_command_port: u32;
   
   acpi_enable:  u8;
   acpi_disable: u8;
   
   s4bios_req: u8;
   
   performance_state_control: u8;
   
   pme1a_block_port: u32;
   pme1b_block_port: u32;
   pmc1a_block_port: u32;
   pmc1b_block_port: u32;
   pmc2_block_port:  u32;
   pmt_block_port:   u32;
   
   gpe0_block_port:  u32;
   gpe1_block_port:  u32;
   
   pm1_event_length:   u8;
   pm1_control_length: u8;
   pm2_control_length: u8;
   pm_timer_length:    u8;
   
   gpe0_length: u8;
   gpe1_length: u8;
   gpe1_base:   u8;
   
   c_state_control: u8;
   
   worst_c2_latency: u16;
   worst_c3_latency: u16;
   
   flush_size:   u16;
   flush_stride: u16;
   
   duty_offset: u8;
   duty_width:  u8;
   
   day_alarm:   u8;
   month_alarm: u8;
   century:     u8;
   
   iapc_boot_arch: u16 #align 1;
   
   _reserved1: u8;
   
   flag: u32;
   
   reset:       Generic_Address;
   reset_value: u8;
   
   arm_boot_arch: u16 #align 1;
   
   fadt_minor_version: u8;
   
   firmware_control_x: u64;
   
   dsdtx: *DSDT;
   
   pm1a_e_block_x: Generic_Address;
   pm1b_e_block_x: Generic_Address;
   pm1a_c_block_x: Generic_Address;
   pm1b_c_block_x: Generic_Address;
   pm2c_block_x:   Generic_Address;
   pmt_block_x:    Generic_Address;
   
   gpe0_block_x: Generic_Address;
   gpe1_block_x: Generic_Address;
   
   sleep_control: Generic_Address;
   sleep_status:  Generic_Address;
   
   hv_identity: u64;
} #no_padding

MADT :: struct {
   #as using _: SDT;
   
   local_apic_address: u32;
   
   flags: enum_flags u32 {
      PCAT_COMPAT :: 1 << 0;
   }
   
   // entries: .. MADT_Entry
}

MADT_Entry :: struct {
   type: enum u8 {
      LOCAL_APIC                        :: 0x00;
      IO_APIC                           :: 0x01;
      INTERRUPT_SOURCE_OVERRIDE         :: 0x02;
      NMI_SOURCE                        :: 0x03;
      LOCAL_APIC_NMI                    :: 0x04;
      LOCAL_APIC_ADDRESS_OVERRIDE       :: 0x05;
      IO_SAPIC                          :: 0x06;
      LOCAL_SAPIC                       :: 0x07;
      PLATFORM_INTERRUPT_SOURCES        :: 0x08;
      LOCAL_X2APIC                      :: 0x09;
      LOCAL_X2APIC_NMI                  :: 0x0A;
      GIC_CPU_INTERFACE                 :: 0x0B;
      GIC_DISTRIBUTOR                   :: 0x0C;
      GIC_MSI_FRAME                     :: 0x0D;
      GIC_REDISTRIBUTOR                 :: 0x0E;
      GIC_INTERRUPT_TRANSLATION_SERVICE :: 0x0F;
      MULTIPROCESSOR_WAKEUP             :: 0x10;
      CORE_PIC                          :: 0x11;
      LEGACY_IO_PIC                     :: 0x12;
      HYPERTRANSPORT_PIC                :: 0x13;
      EXTEND_IO_PIC                     :: 0x14;
      MSI_PIC                           :: 0x15;
      BRIDGE_IO_PIC                     :: 0x16;
      LOW_PIN_COUNT_PIC                 :: 0x17;
   }
   
   length: u8;
} #no_padding

Local_APIC_Override :: struct {
   #as using _: MADT_Entry;
   
   _reserved: [2] u8;
   
   local_apic_address: *Local_APIC #align 4;
} #no_padding

DSDT :: struct {
   #as using _: SDT;
} #no_padding

SDT :: struct {
   signature: [4] u8;
   length: u32;
   revision: u8;
   checksum: u8;
   
   oem_id: [6] u8;
   oem_table_id: [8] u8;
   oem_revision: u32;
   
   creator_id: u32;
   creator_revision: u32;
} #no_padding

Local_APIC :: struct {
   _reserved0: u32 #align 16;
   _reserved1: u32 #align 16;
   
   id:      u32 #align 16; // Read/Write
   version: u32 #align 16; // Read-Only
   
   _reserved4: u32 #align 16;
   _reserved5: u32 #align 16;
   _reserved6: u32 #align 16;
   _reserved7: u32 #align 16;
   
   task_priority:        u32 #align 16; // Read/Write
   arbitration_priority: u32 #align 16; // Read-Only
   processor_priority:   u32 #align 16; // Read-Only
   
   eoi: u32 #align 16; // Write-Only
   
   remote_read: u32 #align 16; // Read-Only
   
   logical_destination: u32 #align 16; // Read/Write
   destination_format:  u32 #align 16; // Read/Write
   
   spurious_interrupt_vector: enum_flags u32 {
      SPURIOUS_VECTOR_MASK :: 0xFF << 0;
      
      ENABLE_APIC              :: 1 << 8;
      FOCUS_PROCESSOR_CHECKING :: 1 << 9;
      SUPPRESS_EOI_BROADCAST   :: 1 << 12;
   } #align 16; // Read/Write
   
   in_service_0: u32 #align 16; // Read-Only
   in_service_1: u32 #align 16; // Read-Only
   in_service_2: u32 #align 16; // Read-Only
   in_service_3: u32 #align 16; // Read-Only
   in_service_4: u32 #align 16; // Read-Only
   in_service_5: u32 #align 16; // Read-Only
   in_service_6: u32 #align 16; // Read-Only
   in_service_7: u32 #align 16; // Read-Only
   
   trigger_mode_0: u32 #align 16; // Read-Only
   trigger_mode_1: u32 #align 16; // Read-Only
   trigger_mode_2: u32 #align 16; // Read-Only
   trigger_mode_3: u32 #align 16; // Read-Only
   trigger_mode_4: u32 #align 16; // Read-Only
   trigger_mode_5: u32 #align 16; // Read-Only
   trigger_mode_6: u32 #align 16; // Read-Only
   trigger_mode_7: u32 #align 16; // Read-Only
   
   interrupt_request_0: u32 #align 16; // Read-Only
   interrupt_request_1: u32 #align 16; // Read-Only
   interrupt_request_2: u32 #align 16; // Read-Only
   interrupt_request_3: u32 #align 16; // Read-Only
   interrupt_request_4: u32 #align 16; // Read-Only
   interrupt_request_5: u32 #align 16; // Read-Only
   interrupt_request_6: u32 #align 16; // Read-Only
   interrupt_request_7: u32 #align 16; // Read-Only
   
   error_status: u32 #align 16; // Read-Only
   
   _reserved29: u32 #align 16;
   _reserved2A: u32 #align 16;
   _reserved2B: u32 #align 16;
   _reserved2C: u32 #align 16;
   _reserved2D: u32 #align 16;
   _reserved2E: u32 #align 16;
   
   lvt_corrected_machine_check_interrupt: u32 #align 16; // Read/Write
   
   interrupt_command_register_0: u32 #align 16; // Read/Write
   interrupt_command_register_1: u32 #align 16; // Read/Write
   
   lvt_timer:                           u32 #align 16; // Read/Write
   lvt_thermal_sensor:                  u32 #align 16; // Read/Write
   lvt_performance_monitoring_counters: u32 #align 16; // Read/Write
   lvt_lint0:                           u32 #align 16; // Read/Write
   lvt_lint1:                           u32 #align 16; // Read/Write
   lvt_error:                           u32 #align 16; // Read/Write
   
   initial_count: u32 #align 16; // Read/Write
   current_count: u32 #align 16; // Read-Only
   
   _reserved3A: u32 #align 16;
   _reserved3B: u32 #align 16;
   _reserved3C: u32 #align 16;
   _reserved3D: u32 #align 16;
   
   divide_configuration: u32 #align 16; // Read/Write
   
   _reserved3F: u32 #align 16;
} #no_padding

MASTER_PIC_COMMAND_PORT: u16 : 0x0020;
SLAVE\_PIC_COMMAND_PORT: u16 : 0x00A0;
MASTER_PIC_DATA_PORT:    u16 : 0x0021;
SLAVE\_PIC_DATA_PORT:    u16 : 0x00A1;

PIC_ICW1 :: enum_flags u8 {
   HAS_ICW4                :: 1 << 0;
   SINGLE_MODE             :: 1 << 1;
   CALL_ADDRESS_INTERVAL_4 :: 1 << 2;
   LEVEL_TRIGGERED_MODE    :: 1 << 3;
   INITIALIZE              :: 1 << 4;
}

PIC_ICW4 :: enum_flags u8 {
   USE_8086_MODE :: 1 << 0;
   AUTO_EOI      :: 1 << 1;
   
   BUFFERED_SLAVE  :: 2 << 2;
   BUFFERED_MASTER :: 3 << 2;
   
   SPECIAL_FULLY_NESTED :: 1 << 4;
}

#scope_file

Generic_Address :: struct {
   address_space: enum u8 {
      SYSTEM_MEMORY                   :: 0x00;
      SYSTEM_IO                       :: 0x01;
      PCI_CONFIGURATOIN               :: 0x02;
      EMBEDDED_CONTROLLER             :: 0x03;
      SM_BSU                          :: 0x04;
      SYSTEM_CMOS                     :: 0x05;
      PCI_BAR_TARGET                  :: 0x06;
      IPMI                            :: 0x07;
      GENERAL_PURPOSE_IO              :: 0x08;
      GENERIC_SERIAL_BUS              :: 0x09;
      PLATFORM_COMMUNICATIONS_CHANNEL :: 0x0A;
      PLATFORM_RUNTIME_MECHANISM      :: 0x0B;
      FUNCTIONAL_FIXED_HARDWARE       :: 0x7F;
   }
   
   bit_count:  u8;
   bit_offset: u8;
   
   access_size: enum u8 {
      UNDEFINED :: 0;
      BYTE      :: 1;
      WORD      :: 2;
      DWORD     :: 3;
      QWORD     :: 4;
   }
   
   address: u64 #align 4;
} #no_padding

calculate_checksum :: (data: *u8, length: int) -> u8 {
   sum: u8 = 0;
   
   for 0..length-1 {
      sum += data[it];
   }
   
   return sum;
}

validate_rsdp :: (using rsdp: *RSDP) {
   assert(rsdp != null, "ThunderOS does not support systems without ACPI tables");
   
   // sum := calculate_checksum(xx rsdp, ifx revision == .RSDP_1_0 then size_of(RSDP) else length);
   sum := calculate_checksum(xx rsdp, size_of(RSDP));
   assert(sum == 0, "The RSDP's checksum does not match");
}

validate_sdt :: (using sdt: *SDT) {
   assert(sdt != null, "Cannot validate a null ACPI table");
   
   sum := calculate_checksum(xx sdt, length);
   assert(sum == 0, "The %'s checksum does not match", get_name(sdt));
}

get_name :: (using sdt: *SDT) -> string {
   name: string;
   name.data = signature.data;
   name.count = signature.count;
   return name;
}

get_name :: (using rsdp: *RSDP) -> string {
   name: string;
   name.data = signature.data;
   name.count = signature.count;
   return name;
}

#import "Util";