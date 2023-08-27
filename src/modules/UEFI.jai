//
// Data Types
//

#if size_of(*void) == 4 {
   uint :: u32;
} else #if size_of(*void) == 8 {
   uint :: u64;
} else {
   #assert false, "Invalid word size";
}

EFI_GUID :: struct {
   data1: u32 #align 8;
   data2: u16;
   data3: u16;
   data4: [8]u8;
}

EFI_Status :: enum int {
   SUCCESS :: 0;
   
   WARNING :: 0 << 63;
   WARN_UNKNOWN_GLYPH    :: WARNING | 1;
   WARN_DELETE_FAILURE   :: WARNING | 2;
   WARN_WRITE_FAILURE    :: WARNING | 3;
   WARN_BUFFER_TOO_SMALL :: WARNING | 4;
   WARN_STALE_DATA       :: WARNING | 5;
   WARN_FILE_SYSTEM      :: WARNING | 6;
   WARN_RESET_REQUIRED   :: WARNING | 7;
   
   ERROR :: 1 << 63;
   LOAD_ERROR           :: ERROR |  1;
   INVALID_PARAMETER    :: ERROR |  2;
   UNSUPPORTED          :: ERROR |  3;
   BAD_BUFFER_SIZE      :: ERROR |  4;
   BUFFER_TOO_SMALL     :: ERROR |  5;
   NOT_READY            :: ERROR |  6;
   DEVICE_ERROR         :: ERROR |  7;
   WRITE_PROTECTED      :: ERROR |  8;
   OUT_OF_RESOURCES     :: ERROR |  9;
   VOLUME_CORRUPTED     :: ERROR | 10;
   VOLUME_FULL          :: ERROR | 11;
   NO_MEDIA             :: ERROR | 12;
   MEDIA_CHANGED        :: ERROR | 13;
   NOT_FOUND            :: ERROR | 14;
   ACCESS_DENIED        :: ERROR | 15;
   NO_RESPONSE          :: ERROR | 16;
   NO_MAPPING           :: ERROR | 17;
   TIMEOUT              :: ERROR | 18;
   NOT_STARTED          :: ERROR | 19;
   ALREADY_STARTED      :: ERROR | 20;
   ABORTED              :: ERROR | 21;
   ICMP_ERROR           :: ERROR | 22;
   TFTP_ERROR           :: ERROR | 23;
   PROTOCOL_ERROR       :: ERROR | 24;
   INCOMPATIBLE_VERSION :: ERROR | 25;
   SECURITY_VIOLATION   :: ERROR | 26;
   CRC_ERROR            :: ERROR | 27;
   END_OF_MEDIA         :: ERROR | 28;
   END_OF_FILE          :: ERROR | 31;
   INVALID_LANGUAGE     :: ERROR | 32;
   COMPROMISED_DATA     :: ERROR | 33;
   IP_ADDRESS_CONFLICT  :: ERROR | 34;
   HTTP_ERROR           :: ERROR | 35;
}

EFI_Handle :: #type,distinct *void;
EFI_Event  :: #type,distinct *void;

EFI_LBA :: #type,distinct u64;
EFI_Physical_Address :: #type,distinct u64;
EFI_Virtual_Address :: #type,distinct u64;

EFI_TPL :: enum uint {
   APPLICATION ::  4;
   CALLBACK    ::  8;
   NOTIFY      :: 16;
   HIGH_LEVEL  :: 31;
}

EFI_MAC_Address  :: #type,distinct [32]u8;
EFI_IPv4_Address :: #type,distinct [ 4]u8;
EFI_IPv6_Address :: #type,distinct [16]u8;
EFI_IP_Address   :: #type,distinct [16]u32;

EFI_Protocol :: struct { }

//
// (4) EFI System Table
//

EFI_Table_Signature :: enum u64 {
   SYSTEM_TABLE     :: 0x5453595320494249;
   BOOT_SERVICES    :: 0x56524553544f4f42;
   RUNTIME_SERVICES :: 0x56524553544e5552;
}

EFI_Table_Revision :: enum u32 {
   SPECIFICATION :: (2 << 16) | 100;
   
   SYSTEM_TABLE     :: SPECIFICATION;
   BOOT_SERVICES    :: SPECIFICATION;
   RUNTIME_SERVICES :: SPECIFICATION;
}

EFI_Table_Header :: struct {
   signature:   EFI_Table_Signature;
   revision:    EFI_Table_Revision;
   header_size: u32;
   crc32:       u32;
   _reserved0:  u32;
}

EFI_System_Table :: struct {
   #as using header: EFI_Table_Header;
   signature = .SYSTEM_TABLE;
   revision  = .SYSTEM_TABLE;
   
   firmware_vendor: *u16;
   firmware_revision: u32;
   
   console_in_handle: EFI_Handle;
   console_in: *EFI_Simple_Text_Input_Protocol;
   
   console_out_handle: EFI_Handle;
   console_out: *EFI_Simple_Text_Output_Protocol;
   
   standard_error_handle: EFI_Handle;
   standard_error: *EFI_Simple_Text_Output_Protocol;
   
   runtime_services: *EFI_Runtime_Services;
   boot_services: *EFI_Boot_Services;
   
   number_of_table_entries: uint;
   configuration_table: *EFI_Configuration_Table;
}

EFI_Configuration_Table :: struct {
   vendor_guid:  EFI_GUID;
   vendor_table: *void;
}

//
// (7) Boot Services
//

EFI_Boot_Services :: struct {
   #as using header: EFI_Table_Header;
   signature = .BOOT_SERVICES;
   revision  = .BOOT_SERVICES;
   
   //
   // Task Priority Services
   //
   raise_tpl:   (new_tpl: EFI_TPL) -> EFI_TPL #c_call #no_context; // EFI 1.0+
   restore_tpl: (old_tpl: EFI_TPL) #c_call #no_context; // EFI 1.0+

   //
   // Memory Services
   //
   allocate_pages: (type: EFI_Allocate_Type, memory_type: EFI_Memory_Type, pages: uint, memory: *EFI_Physical_Address) -> EFI_Status #c_call #no_context; // EFI 1.0+
   free_pages:     (memory: EFI_Physical_Address, pages: uint) -> EFI_Status #c_call #no_context; // EFI 1.0+
   get_memory_map: (memory_map_size: *uint, memory_map: *EFI_Memory_Descriptor, map_key: *uint, descriptor_size: *uint, descriptor_version: *u32) -> EFI_Status #c_call #no_context; // EFI 1.0+
   allocate_pool:  (pool_type: EFI_Memory_Type, size: uint, buffer: **void) -> EFI_Status #c_call #no_context; // EFI 1.0+
   free_pool:      (buffer: *void) -> EFI_Status #c_call #no_context; // EFI 1.0+
   
   //
   // Event & Timer Services
   //
   create_event:   (type: EFI_Event_Type, notify_tpl: EFI_TPL, notify_function: EFI_Event_Notify, notify_context: *void, event: *EFI_Event) -> EFI_Status #c_call #no_context; // EFI 1.0+
   set_timer:      (event: EFI_Event, type: EFI_Timer_Delay, trigger_time: u64) -> EFI_Status #c_call #no_context; // EFI 1.0+
   wait_for_event: (number_of_events: uint, events: *EFI_Event, index: *uint) -> EFI_Status #c_call #no_context; // EFI 1.0+
   signal_event:   (event: EFI_Event) -> EFI_Status #c_call #no_context; // EFI 1.0+
   close_event:    (event: EFI_Event) -> EFI_Status #c_call #no_context; // EFI 1.0+
   check_event:    (event: EFI_Event) -> EFI_Status #c_call #no_context; // EFI 1.0+
   
   //
   // Protocol Handler Services
   //
   install_protocol_interface:   (handle: *EFI_Handle, protocol: *EFI_GUID, interface_type: EFI_Interface_Type, protocol_interface: *EFI_Protocol) -> EFI_Status #c_call #no_context; // EFI 1.0+
   reinstall_protocol_interface: (handle: EFI_Handle, protocol: *EFI_GUID, old_interface: *EFI_Protocol, new_interface: *EFI_Protocol) -> EFI_Status #c_call #no_context; // EFI 1.0+
   uninstall_protocol_interface: (handle: EFI_Handle, protocol: *EFI_GUID, protocol_interface: *EFI_Protocol) -> EFI_Status #c_call #no_context; // EFI 1.0+
   handle_protocol:              (handle: EFI_Handle, protocol: *EFI_GUID, protocol_interface: **EFI_Protocol) -> EFI_Status #c_call #no_context; // EFI 1.0+
   _reserved2:                   *void; // EFI 1.0+
   register_protocol_notify:     (protocol: *EFI_GUID, event: EFI_Event, registration: **void) -> EFI_Status #c_call #no_context; // EFI 1.0+
   locate_handle:                (search_type: EFI_Locate_Search_Type, protocol: *EFI_GUID, search_key: *void, buffer_size: *uint, buffer: *EFI_Handle) -> EFI_Status #c_call #no_context; // EFI 1.0+
   locate_device_path:           (protocol: *EFI_GUID, device_path: **EFI_Device_Path_Protocol, device: *EFI_Handle) -> EFI_Status #c_call #no_context; // EFI 1.0+
   install_configuration_table:  (guid: *EFI_GUID, table: *void) -> EFI_Status #c_call #no_context; // EFI 1.0+

   //
   // Image Services
   //
   load_image:         (boot_policy: bool, parent_image_handle: EFI_Handle, device_path: *EFI_Device_Path_Protocol, source_buffer: *void, source_size: uint, image_handle: *EFI_Handle) -> EFI_Status #c_call #no_context; // EFI 1.0+
   start_image:        (image_handle: EFI_Handle, exit_data_size: *uint, exit_data: **u16) -> EFI_Status #c_call #no_context; // EFI 1.0+
   exit:               (image_handle: EFI_Handle, exit_status: EFI_Status, exit_data_size: uint, exit_data: *u16) -> EFI_Status #c_call #no_context; // EFI 1.0+
   unload_image:       (image_handle: EFI_Handle) -> EFI_Status #c_call #no_context; // EFI 1.0+
   exit_boot_services: (image_handle: EFI_Handle, map_key: uint) -> EFI_Status #c_call #no_context; // EFI 1.0+

   //
   // Miscellaneous Services
   //
   get_next_monotonic_count: (count: *u64) -> EFI_Status #c_call #no_context; // EFI 1.0+
   stall:                    (microseconds: uint) -> EFI_Status #c_call #no_context; // EFI 1.0+
   set_watchdog_timer:       (timeout: uint, watchdog_code: u64, data_size: uint, watchdog_data: *u16) -> EFI_Status #c_call #no_context; // EFI 1.0+

   //
   // DriverSupport Services
   //
   connect_controller:    (controller_handle: EFI_Handle, driver_image_handle: *EFI_Handle, remaining_device_path: *EFI_Device_Path_Protocol, recursive: bool) -> EFI_Status #c_call #no_context; // EFI 1.1
   disconnect_controller: (controller_handle: EFI_Handle, driver_image_handle: EFI_Handle, child_handle: EFI_Handle) -> EFI_Status #c_call #no_context; // EFI 1.1+

   //
   // Open and Close Protocol Services
   //
   open_protocol:             (handle: EFI_Handle, protocol: *EFI_GUID, protocol_interface: **EFI_Protocol, agent_handle: EFI_Handle, controller_handle: EFI_Handle, attributes: EFI_Open_Protocol_Attributes) -> EFI_Status #c_call #no_context; // EFI 1.1+
   close_protocol:            (handle: EFI_Handle, protocol: *EFI_GUID, agent_handle: EFI_Handle, controller_handle: EFI_Handle) -> EFI_Status #c_call #no_context; // EFI 1.1+
   open_protocol_information: (handle: EFI_Handle, protocol: *EFI_GUID, entry_buffer: **EFI_Open_Protocol_Information_Entry, entry_count: *uint) -> EFI_Status #c_call #no_context; // EFI 1.1+

   //
   // Library Services
   //
   protocols_per_handle:                   (handle: EFI_Handle, protocol_buffer: ***EFI_GUID, protocol_buffer_count: *uint) -> EFI_Status #c_call #no_context; // EFI 1.1+
   locate_handle_buffer:                   (search_type: EFI_Locate_Search_Type, protocol: *EFI_GUID, search_key: *void, handle_count: *uint, buffer: **EFI_Handle) -> EFI_Status #c_call #no_context; // EFI 1.1+
   locate_protocol:                        (protocol: *EFI_GUID, registration: *void, protocol_interface: **EFI_Protocol) -> EFI_Status #c_call #no_context; // EFI 1.1+
   install_multiple_protocol_interfaces:   (handle: *EFI_Handle, protocols: ..*void) -> EFI_Status #c_call #no_context; // EFI 1.1+
   uninstall_multiple_protocol_interfaces: (handle: EFI_Handle, protocols: ..*void) -> EFI_Status #c_call #no_context; // EFI 1.1+

   //
   // 32-bit CRC Services
   //
   calculate_crc32: (data: *void, data_size: uint, crc32: *u32) -> EFI_Status #c_call #no_context; // EFI 1.1+

   //
   // Miscellaneous Services
   //
   copy_mem:        (destination: *void, source: *void, length: uint) #c_call #no_context; // EFI 1.1+
   set_mem:         (buffer: *void, size: uint, value: u8) #c_call #no_context; // EFI 1.1+
   create_event_ex: (type: EFI_Event_Type, notify_tpl: EFI_TPL, notify_function: EFI_Event_Notify, notify_context: *void, event_group: *EFI_GUID, event: *EFI_Event) -> EFI_Status #c_call #no_context; // UEFI 2.0+
}

EFI_Allocate_Type :: enum u32 {
   ANY_PAGES   :: 0;
   MAX_ADDRESS :: 1;
   ADDRESS     :: 2;
}

EFI_Memory_Type :: enum u32 {
   RESERVED              ::  0;
   LOADER_CODE           ::  1;
   LOADER_DATA           ::  2;
   BOOT_SERVICES_CODE    ::  3;
   BOOT_SERVICES_DATA    ::  4;
   RUNTIME_SERVICES_CODE ::  5;
   RUNTIME_SERVICES_DATA ::  6;
   CONVENTIONAL          ::  7;
   UNUSABLE              ::  8;
   ACPI_RECLAIM          ::  9;
   ACPI_NVS              :: 10;
   MMIO                  :: 11;
   MMIO_PORT_SPACE       :: 12;
   PAL_CODE              :: 13;
   PERSISTENT            :: 14;
   UNACCEPTED            :: 15;
}

EFI_Memory_Attribute :: enum_flags u64 {
   UNCACHEABLE     :: 1 << 0;
   WRITE_COMBINING :: 1 << 1;
   WRITE_THROUGH   :: 1 << 2;
   WRITE_BACK      :: 1 << 3;
   EXPORTED        :: 1 << 4;
   
   WRITE_PROTECTION   :: 1 << 12;
   READ_PROTECTION    :: 1 << 13;
   EXECUTE_PROTECTION :: 1 << 14;
   NONVOLATILE        :: 1 << 15;
   MORE_RELIABLE      :: 1 << 16;
   READ_ONLY          :: 1 << 17;
   SPECIFIC_PURPOSE   :: 1 << 18;
   CPU_CRYPTO         :: 1 << 19;
   
   MEMORY_ISA_MASK :: 0xFFFF << 44;
   
   ISA_VALID :: 1 << 62;
   RUNTIME   :: 1 << 63;
}

EFI_Memory_Descriptor :: struct {
   type:            EFI_Memory_Type;
   physical_start:  EFI_Physical_Address;
   virtual_start:   EFI_Virtual_Address;
   number_of_pages: u64;
   attribute:       EFI_Memory_Attribute;
}

EFI_Event_Notify :: #type (event: EFI_Event, event_context: *void);

EFI_Event_Type :: enum u32 {
   NOTIFY_WAIT                   :: 0x00000100;
   NOTIFY_SIGNAL                 :: 0x00000200;
   RUNTIME                       :: 0x40000000;
   TIMER                         :: 0x80000000;
   
   SIGNAL_EXIT_BOOT_SERVICES     :: 0x00000201;
   SIGNAL_VIRTUAL_ADDRESS_CHANGE :: 0x60000202;
}

EFI_Timer_Delay :: enum u32 {
   CANCEL   :: 0;
   PERIODIC :: 1;
   RELATIVE :: 2;
}

EFI_Interface_Type :: enum u32 {
   NATIVE :: 0;
}

EFI_Locate_Search_Type :: enum u32 {
   ALL_HANDLES :: 0;
   BY_REGISTER_NOTIFY :: 1;
   BY_PROTOCOL :: 2;
}

EFI_Open_Protocol_Attributes :: enum_flags u32 {
   BY_HANDLE_PROTOCOL  :: 1 << 0;
   GET_PROTOCOL        :: 1 << 1;
   TEST_PROTOCOL       :: 1 << 2;
   BY_CHILD_CONTROLLER :: 1 << 3;
   BY_DRIVER           :: 1 << 4;
   EXCLUSIVE           :: 1 << 5;
}

EFI_Open_Protocol_Information_Entry :: struct {
   agent_handle: EFI_Handle;
   controller_handle: EFI_Handle;
   attributes: u32;
   open_count: u32;
}

//
// (8) Runtime Services
//

EFI_Runtime_Services :: struct {
   #as using header: EFI_Table_Header;
   signature = .RUNTIME_SERVICES;
   revision  = .RUNTIME_SERVICES;
   
   //
   // Time Services
   //
   get_time:        (time: *EFI_Time, capabilities: *EFI_Time_Capabilities) -> EFI_Status #c_call #no_context;
   set_time:        (time: *EFI_Time) -> EFI_Status #c_call #no_context;
   get_wakeup_time: (enabled: *bool, pending: *bool, time: *EFI_Time) -> EFI_Status #c_call #no_context;
   set_wakeup_time: (enable: bool, time: *EFI_Time) -> EFI_Status #c_call #no_context;

   //
   // Virtual Memory Services
   //
   set_virtual_address_map: (memory_map_size: uint, descriptor_size: uint, descriptor_version: u32, virtual_map: *EFI_Memory_Descriptor) -> EFI_Status #c_call #no_context;
   convert_pointer:         (debug_disposition: EFI_Debug_Disposition, address: **void) -> EFI_Status #c_call #no_context;

   //
   // Variable Services
   //
   get_variable:           (variable_name: *u16, vendor_guid: *EFI_GUID, attributes: *EFI_Variable_Attributes, data_size: *uint, data: *void) -> EFI_Status #c_call #no_context;
   get_next_variable_name: (variable_name_size: *uint, variable_name: *u16, vendor_guid: *EFI_GUID) -> EFI_Status #c_call #no_context;
   set_variable:           (variable_name: *u16, vendor_guid: *EFI_GUID, attributes: EFI_Variable_Attributes, data_size: uint, data: *void) -> EFI_Status #c_call #no_context;

   //
   // Miscellaneous Services
   //
   get_next_high_monotonic_count: (high_count: *u32) -> EFI_Status #c_call #no_context;
   reset_system:                  (reset_type: EFI_Reset_Type, reset_status: EFI_Status, data_size: uint, reset_data: *void) -> EFI_Status #c_call #no_context;

   //
   // Capsule Services
   //
   update_capsule:             (capsule_header_array: **EFI_Capsule_Header, capsule_count: uint, scatter_gather_list: EFI_Physical_Address) -> EFI_Status #c_call #no_context; // UEFI 2.0+
   query_capsule_capabilities: (capsule_header_array: **EFI_Capsule_Header, capsule_count: uint, maximum_capsule_size: *u64, reset_type: *EFI_Reset_Type) -> EFI_Status #c_call #no_context; // UEFI 2.0+

   //
   // Miscellaneous Services
   //
   query_variable_info: (attributes: EFI_Variable_Attributes, maximum_variable_storage_size: *u64, remaining_variable_storage_size: *u64, maximum_variable_size: *u64) -> EFI_Status #c_call #no_context; // UEFI 2.0+
}

EFI_Variable_Attributes :: enum u32 {
   NON_VOLATILE                          :: 1 << 0;
   BOOTSERVICE_ACCESS                    :: 1 << 1;
   RUNTIME_ACCESS                        :: 1 << 2;
   HARDWARE_ERROR_RECORD                 :: 1 << 3;
   AUTHENTICATED_WRITE_ACCESS            :: 1 << 4;
   TIME_BASED_AUTHENTICATED_WRITE_ACCESS :: 1 << 5;
   APPEND_WRITE                          :: 1 << 6;
   ENHANCED_AUTHENTICATED_ACCESS         :: 1 << 7;
}

EFI_Time :: struct {
   year:       u16; // 1900 - 9999
   month:      u8;  // 1 - 12
   day:        u8;  // 1 - 31
   hour:       u8;  // 0 - 23
   minute:     u8;  // 0 - 59
   second:     u8;  // 0 - 59
   _pad1:      u8;
   nanosecond: u32; // 0 - 999,999,999
   timezone:   s16; // --1440 to 1440 or 2047
   daylight:   enum_flags u8 {
      ADJUST_DAYLIGHT :: 1 << 0;
      IN_DAYLIGHT :: 1 << 1;
   }
   _pad2:      u8;
   
   UNSPECIFIED_TIMEZONE :: 0x07FF;
}

EFI_Time_Capabilities :: struct {
   resolution: u32;
   accuracy: u32;
   sets_to_zero: bool;
}

EFI_Debug_Disposition :: enum_flags uint {
   OPTIONAL :: 1 << 0;
}

EFI_Reset_Type :: enum u32 {
   COLD              :: 0;
   WARM              :: 1;
   SHUTDOWN          :: 2;
   PLATFORM_SPECIFIC :: 3;
}

EFI_Capsule_Header :: struct {
   capsule_guid: EFI_GUID;
   header_size: u32;
   flags: EFI_Capsule_Flags;
   capsule_image_size: u32;
}

EFI_Capsule_Flags :: enum_flags u32 {
   PERSIST_ACROSS_RESET  :: 1 << 16;
   POPULATE_SYSTEM_TABLE :: 1 << 17;
   INITIATE_RESET        :: 1 << 18;
}

//
// (9) Loaded Image Protocols
//

EFI_Loaded_Image_Protocol :: struct {
   revision: u32 = 0x00001000;
   
   parent_handle: EFI_Handle;
   
   system_table: *EFI_System_Table;
   
   device_handle: EFI_Handle;
   file_path: *EFI_Device_Path_Protocol;
   
   reserved: *void;
   
   load_options_size: u32;
   load_options: *void;
   
   image_base: *void;
   image_size: u64;
   
   image_code_type: EFI_Memory_Type;
   image_data_type: EFI_Memory_Type;
   
   unload: (image_handle: EFI_Handle) -> EFI_Status #c_call #no_context;
   
   GUID :: EFI_GUID.{0x5b1b31a1, 0x9562, 0x11d2, .[0x8e, 0x3f, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b]};
}

EFI_Loaded_Image_Device_Path_Protocol :: struct {
   revision: u32 = 0x00001000;
   
   parent_handle: EFI_Handle;
   
   system_table: *EFI_System_Table;
   
   device_handle: EFI_Handle;
   file_path: *EFI_Device_Path_Protocol;
   
   reserved: *void;
   
   load_options_size: u32;
   load_options: *void;
   
   image_base: *void;
   image_size: u64;
   
   image_code_type: EFI_Memory_Type;
   image_data_type: EFI_Memory_Type;
   
   unload: (image_handle: EFI_Handle) -> EFI_Status #c_call #no_context;
   
   GUID :: EFI_GUID.{0xbc62157e, 0x3e33, 0x4fec, .[0x99, 0x20, 0x2d, 0x3b, 0x36, 0xd7, 0x50, 0xdf]};
}

//
// (10) Device Path Protocols
//

EFI_Device_Path_Protocol :: struct {
   type: enum u8 {
      HARDWARE                :: 0x01;
      ACPI                    :: 0x02;
      MESSAGING               :: 0x03;
      MEDIA                   :: 0x04;
      BIOS_BOOT_SPECIFICATION :: 0x05;
      END_OF_HARDWARE         :: 0x7F;
   }
   sub_type: u8;
   length: [2]u8;
   
   GUID :: EFI_GUID.{0x09576e91, 0x6d3f, 0x11d2, .[0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b]};
}

//
// (12) Console Protocols
//

EFI_Simple_Text_Input_Ex_Protocol :: struct {
   reset:                 (this: *This, extended_verification: bool) -> EFI_Status #c_call #no_context;
   read_key_stroke_ex:    (this: *This, key_data: *EFI_Key_Data) -> EFI_Status #c_call #no_context;
   wait_for_key_ex:       EFI_Event;
   set_state:             (this: *This, key_toggle_state: *EFI_Key_Toggle_State) -> EFI_Status #c_call #no_context;
   register_key_notify:   (this: *This, key_data: *EFI_Key_Data, key_notification_function: EFI_Key_Notify_Function, notify_handle: *EFI_Handle) -> EFI_Status #c_call #no_context;
   unregister_key_notify: (this: *This, notification_handle: EFI_Handle) -> EFI_Status #c_call #no_context;
   
   GUID :: EFI_GUID.{0xdd9e7534, 0x7762, 0x4698, .[0x8c, 0x14, 0xf5, 0x85, 0x17, 0xa6, 0x25, 0xaa]};
   This :: #this;
   
   EFI_Key_Notify_Function :: #type (key_data: *EFI_Key_Data) -> EFI_Status #c_call #no_context;
}

EFI_Simple_Text_Input_Protocol :: struct {
   reset:           (this: *This, extended_verification: bool) -> EFI_Status #c_call #no_context;
   read_key_stroke: (this: *This, key: *EFI_Input_Key) -> EFI_Status #c_call #no_context;
   wait_for_key:    EFI_Event;
   
   GUID :: EFI_GUID.{0x387477c1, 0x69c7, 0x11d2, .[0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b]};
   This :: #this;
}

EFI_Simple_Text_Output_Protocol :: struct {
   reset:               (this: *This, extended_verification: bool) -> EFI_Status #c_call #no_context;
   output_string:       (this: *This, string: *u16) -> EFI_Status #c_call #no_context;
   test_string:         (this: *This, string: *u16) -> EFI_Status #c_call #no_context;
   query_mode:          (this: *This, mode_number: uint, columns: *uint, rows: *uint) -> EFI_Status #c_call #no_context;
   set_mode:            (this: *This, mode_number: uint) -> EFI_Status #c_call #no_context;
   set_attribute:       (this: *This, attribute: Attribute) -> EFI_Status #c_call #no_context;
   clear_screen:        (this: *This) -> EFI_Status #c_call #no_context;
   set_cursor_position: (this: *This, column: uint, row: uint) -> EFI_Status #c_call #no_context;
   enable_cursor:       (this: *This, visible: bool) -> EFI_Status #c_call #no_context;
   mode:                *Simple_Text_Output_Mode;
   
   GUID :: EFI_GUID.{0x387477c2, 0x69c7, 0x11d2, .[0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b]};
   This :: #this;
   
   Simple_Text_Output_Mode :: struct {
      max_mode:       s32;
      mode:           s32;
      attribute:      s32;
      cursor_column:  s32;
      cursor_row:     s32;
      cursor_visible: bool;
   }
   
   Attribute :: enum uint {
      BRIGHT :: 0x8;
      
      BLACK        :: 0x0;
      BLUE         :: 0x1;
      GREEN        :: 0x2;
      CYAN         :: 0x3;
      RED          :: 0x4;
      MAGENTA      :: 0x5;
      BROWN        :: 0x6;
      LIGHTGRAY    :: 0x7;
      DARKGRAY     :: BLACK     | BRIGHT;
      LIGHTBLUE    :: BLUE      | BRIGHT;
      LIGHTGREEN   :: GREEN     | BRIGHT;
      LIGHTCYAN    :: CYAN      | BRIGHT;
      LIGHTRED     :: RED       | BRIGHT;
      LIGHTMAGENTA :: MAGENTA   | BRIGHT;
      YELLOW       :: BROWN     | BRIGHT;
      WHITE        :: LIGHTGRAY | BRIGHT;
      
      BACKGROUND_BLACK     :: BLACK     << 4;
      BACKGROUND_BLUE      :: BLUE      << 4;
      BACKGROUND_GREEN     :: GREEN     << 4;
      BACKGROUND_CYAN      :: CYAN      << 4;
      BACKGROUND_RED       :: RED       << 4;
      BACKGROUND_MAGENTA   :: MAGENTA   << 4;
      BACKGROUND_BROWN     :: BROWN     << 4;
      BACKGROUND_LIGHTGRAY :: LIGHTGRAY << 4;
   }
}

EFI_Key_Data :: struct {
   key:       EFI_Input_Key;
   key_state: EFI_Key_State;
}

EFI_Input_Key :: struct {
   scan_code:    u16;
   unicode_char: u16;
}

EFI_Key_State :: struct {
   key_shift_state: EFI_Key_Shift_State;
   key_toggle_state: EFI_Key_Toggle_State;
}

EFI_Key_Shift_State :: enum_flags u32 {
   RIGHT_SHIFT_PRESSED   :: 1 << 0;
   LEFT_SHIFT_PRESSED    :: 1 << 1;
   RIGHT_CONTROL_PRESSED :: 1 << 2;
   LEFT_CONTROL_PRESSED  :: 1 << 3;
   RIGHT_ALT_PRESSED     :: 1 << 4;
   LEFT_ALT_PRESSED      :: 1 << 5;
   RIGHT_LOGO_PRESSED    :: 1 << 6;
   LEFT_LOGO_PRESSED     :: 1 << 7;
   MENU_KEY_PRESSED      :: 1 << 8;
   SYS_REQ_PRESSED       :: 1 << 9;
   
   SHIFT_STATE_VALID :: 1 << 31;
}

EFI_Key_Toggle_State :: enum_flags u8 {
   SCROLL_LOCK_ACTIVE :: 1 << 0;
   NUM_LOCK_ACTIVE    :: 1 << 1;
   CAPS_LOCK_ACTIVE   :: 1 << 2;
   
   KEY_STATE_EXPOSED  :: 1 << 6;
   TOGGLE_STATE_VALID :: 1 << 7;
}

//
// (13) Media Access Protocols
// TODO incomplete
//

EFI_Simple_File_System_Protocol :: struct {
   revision: enum u64 {
      REVISION_1 :: 0x00010000;
   } = .REVISION_1;
   
   open_volume: (this: *This, root: **EFI_File_Protocol) -> EFI_Status #c_call #no_context;
   
   GUID :: EFI_GUID.{0x0964e5b22, 0x6459, 0x11d2, .[0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b]};
   This :: #this;
}

EFI_File_Protocol :: struct {
   revision: enum u64 {
      REVISION_1 :: 0x00010000;
      REVISION_2 :: 0x00020000;
   } = .REVISION_2;
   
   open:         (this: *This, new_handle: **EFI_File_Protocol, file_name: *u16, open_mode: EFI_File_Mode, attributes: EFI_File_Attributes) -> EFI_Status #c_call #no_context;
   close:        (this: *This) -> EFI_Status #c_call #no_context;
   
   delete:       (this: *This) -> EFI_Status #c_call #no_context;
   read:         (this: *This, buffer_size: *uint, buffer: *u8) -> EFI_Status #c_call #no_context;
   write:        (this: *This, buffer_size: *uint, buffer: *u8) -> EFI_Status #c_call #no_context;
   
   get_position: (this: *This, position: *u64) -> EFI_Status #c_call #no_context;
   set_position: (this: *This, position: u64) -> EFI_Status #c_call #no_context;
   
   get_info:     (this: *This, information_type: *EFI_GUID, buffer_size: *uint, buffer: *u8) -> EFI_Status #c_call #no_context;
   set_info:     (this: *This, information_type: *EFI_GUID, buffer_size: uint, buffer: *u8) -> EFI_Status #c_call #no_context;
   
   flush:        (this: *This) -> EFI_Status #c_call #no_context;
   
   open_ex:      (this: *This, new_handle: **EFI_File_Protocol, file_name: *u16, open_mode: EFI_File_Mode, attributes: EFI_File_Attributes, token: *EFI_File_IO_Token) -> EFI_Status #c_call #no_context; // Revision 2
   read_ex:      (this: *This, token: *EFI_File_IO_Token) -> EFI_Status #c_call #no_context; // Revision 2
   write_ex:     (this: *This, token: *EFI_File_IO_Token) -> EFI_Status #c_call #no_context; // Revision 2
   flush_ex:     (this: *This, token: *EFI_File_IO_Token) -> EFI_Status #c_call #no_context; // Revision 2
   
   This :: #this;
}

EFI_File_Info :: struct {
   size: u64;
   file_size: u64;
   physical_size: u64;
   
   create_time: EFI_Time;
   last_access_time: EFI_Time;
   modification_time: EFI_Time;
   
   attribute: EFI_File_Attributes;
   
   file_name: []u16;
   
   GUID :: EFI_GUID.{0x09576e92, 0x6d3f, 0x11d2, .[0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b]};
}

EFI_File_Mode :: enum_flags u64 {
   READ   :: 1 << 0;
   WRITE  :: 1 << 1;
   CREATE :: 1 << 63;
}

EFI_File_Attributes :: enum_flags u64 {
   READ_ONLY  :: 1 << 0;
   HIDDEN     :: 1 << 1;
   SYSTEM     :: 1 << 2;
   DIRECTORY  :: 1 << 4;
   ARCHIVE    :: 1 << 5;
}

EFI_File_IO_Token :: struct {
   event: EFI_Event;
   status: EFI_Status;
   
   buffer_size: uint;
   buffer: *u8;
}