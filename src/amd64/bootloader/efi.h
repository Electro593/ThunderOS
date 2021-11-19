/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                         **
**  Author: Aria Seiler                                                    **
**                                                                         **
**  This program is in the public domain. There is no implied warranty,    **
**  so use it at your own risk.                                            **
**                                                                         **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define EFI_API API

#define EFI_2_90_SYSTEM_TABLE_REVISION ((2<<16) | (90))
#define EFI_2_80_SYSTEM_TABLE_REVISION ((2<<16) | (80))
#define EFI_2_70_SYSTEM_TABLE_REVISION ((2<<16) | (70))
#define EFI_2_60_SYSTEM_TABLE_REVISION ((2<<16) | (60))
#define EFI_2_50_SYSTEM_TABLE_REVISION ((2<<16) | (50))
#define EFI_2_40_SYSTEM_TABLE_REVISION ((2<<16) | (40))
#define EFI_2_31_SYSTEM_TABLE_REVISION ((2<<16) | (31))
#define EFI_2_30_SYSTEM_TABLE_REVISION ((2<<16) | (30))
#define EFI_2_20_SYSTEM_TABLE_REVISION ((2<<16) | (20))
#define EFI_2_10_SYSTEM_TABLE_REVISION ((2<<16) | (10))
#define EFI_2_00_SYSTEM_TABLE_REVISION ((2<<16) | (00))
#define EFI_1_10_SYSTEM_TABLE_REVISION ((1<<16) | (10))
#define EFI_1_02_SYSTEM_TABLE_REVISION ((1<<16) | (02))
#define EFI_SYSTEM_TABLE_REVISION EFI_2_90_SYSTEM_TABLE_REVISION
#define EFI_SYSTEM_TABLE_SIGNATURE 0x5453595320494249
#define EFI_SPECIFICATION_VERSION EFI_SYSTEM_TABLE_REVISION
#define EFI_BOOT_SERVICES_REVISION EFI_SPECIFICATION_VERSION
#define EFI_BOOT_SERVICES_SIGNATURE 0x56524553544f4f42
#define EFI_RUNTIME_SERVICES_REVISION EFI_SPECIFICATION_VERSION
#define EFI_RUNTIME_SERVICES_SIGNATURE 0x56524553544e5552

#define EFI_GUID(Data1, Data2, Data3, ...)  (efi_guid){Data1, Data2, Data3, {__VA_ARGS__}}
#define EFI_SIMPLE_TEXT_INPUT_PROTOCOL_GUID  EFI_GUID(0x387477c1, 0x69c7, 0x11d2, 0x8e,0x39,0x00,0xa0,0xc9,0x69,0x72,0x3b)
#define EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL_GUID EFI_GUID(0x387477c2, 0x69c7, 0x11d2, 0x8e,0x39,0x00,0xa0,0xc9,0x69,0x72,0x3b)
#define EFI_DEVICE_PATH_PROTOCOL_GUID        EFI_GUID(0x09576e91, 0x6d3f, 0x11d2, 0x8e,0x39,0x00,0xa0,0xc9,0x69,0x72,0x3b)


typedef u32  efi_status;
typedef vptr efi_handle;
typedef vptr efi_event;
typedef u64  efi_tpl;
typedef u64  efi_physical_address;
typedef u64  efi_virtual_address;



typedef void (EFI_API *func_EFI_EventNotify)(IN efi_event Event, IN vptr Context);



typedef enum efi_status_enum {
    EFI_Status_Success   = 0,
} efi_status_enum;

typedef enum efi_tpl_enum {
    EFI_TPL_Application =  4,
    EFI_TPL_Callback    =  8,
    EFI_TPL_Notify      = 16,
    EFI_TPL_HighLevel   = 31,
} efi_tpl_enum;

typedef enum efi_allocate_type {
    EFI_AllocateType_AnyPages,
    EFI_AllocateType_MaxAddress,
    EFI_AllocateType_Address,
    EFI_AllocateType_Max
} efi_allocate_type;

typedef enum efi_memory_type {
    EFI_MemoryType_Reserved,
    EFI_MemoryType_LoaderCode,
    EFI_MemoryType_LoaderData,
    EFI_MemoryType_BootServicesCode,
    EFI_MemoryType_BootServicesData,
    EFI_MemoryType_RuntimeServicesCode,
    EFI_MemoryType_RuntimeServicesData,
    EFI_MemoryType_Conventional,
    EFI_MemoryType_Unusable,
    EFI_MemoryType_ACPIReclaim,
    EFI_MemoryType_ACPIMemoryNVS,
    EFI_MemoryType_MappedIO,
    EFI_MemoryType_MappedIOPortSpace,
    EFI_MemoryType_PalCode,
    EFI_MemoryType_Persistent,
    EFI_MemoryType_Unaccepted,
    EFI_MemoryType_Max
} efi_memory_type;

typedef enum efi_timer_delay {
    EFI_TimerDelay_Cancel,
    EFI_TimerDelay_Periodic,
    EFI_TimerDelay_Relative,
} efi_timer_delay;

typedef enum efi_interface_type {
    EFI_InterfaceType_Native,
} efi_interface_type;

typedef enum efi_locate_search_type {
    EFI_LocateSearch_AllHandles,
    EFI_LocateSearch_ByRegisterNotify,
    EFI_LocateSearch_ByProtocol,
} efi_locate_search_type;

typedef enum efi_reset_type {
    EFI_ResetType_Cold,
    EFI_ResetType_Warm,
    EFI_ResetType_Shutdown,
    EFI_ResetType_PlatformSpecific,
} efi_reset_type;



typedef struct efi_guid {
    u32 Data1;
    u16 Data2;
    u16 Data3;
    u08 Data4[8];
} efi_guid;

typedef struct efi_table_header {
    u64 Signature;
    u32 Revision;
    u32 HeaderSize;
    u32 CRC32;
    u32 Reserved;
} efi_table_header;

typedef struct efi_memory_descriptor {
    u32 Type;
    efi_physical_address PhysicalStart;
    efi_virtual_address VirtualStart;
    u64 NumberOfPages;
    u64 Attribute;
} efi_memory_descriptor;

typedef struct efi_input_key {
    u16 ScanCode;
    c16 UnicodeChar;
} efi_input_key;

typedef struct efi_open_protocol_information_entry {
   efi_handle AgentHandle;
   efi_handle ControllerHandle;
   u32 Attributes;
   u32 OpenCount;
} efi_open_protocol_information_entry;

typedef struct efi_time {
    u16 Year;   // 1900 – 9999
    u08 Month;  // 1 – 12
    u08 Day;    // 1 – 31
    u08 Hour;   // 0 – 23
    u08 Minute; // 0 – 59
    u08 Second; // 0 – 59
    u08 Pad1;
    u32 Nanosecond; // 0 – 999,999,999
    s16 TimeZone;   // -1440 to 1440 or 2047
    u08 Daylight;
    u08 Pad2;
} efi_time;

typedef struct efi_time_capabilities {
    u32 Resolution;
    u32 Accuracy;
    b08 SetsToZero;
} efi_time_capabilities;

typedef struct efi_capsule_header {
    efi_guid CapsuleGuid;
    u32 HeaderSize;
    u32 Flags;
    u32 CapsuleImageSize;
} efi_capsule_header;

typedef struct efi_simple_text_input_protocol {
    efi_status (EFI_API *Reset)         (IN struct efi_simple_text_input_protocol *This, IN b08 ExtendedVerification);
    efi_status (EFI_API *ReadKeyStroke) (IN struct efi_simple_text_input_protocol *This, OUT efi_input_key *Key);
    
    efi_event WaitForKey;
} efi_simple_text_input_protocol;

typedef struct efi_simple_text_output_protocol {
    efi_status (EFI_API *Reset)             (IN struct efi_simple_text_output_protocol *This, IN b08 ExtendedVerification);
    efi_status (EFI_API *OutputString)      (IN struct efi_simple_text_output_protocol *This, IN c16 *String);
    efi_status (EFI_API *TestString)        (IN struct efi_simple_text_output_protocol *This, IN c16 *String);
    efi_status (EFI_API *QueryMode)         (IN struct efi_simple_text_output_protocol *This, IN u64 ModeNumber, OUT u64 *Columns, OUT u64 *Rows);
    efi_status (EFI_API *SetMode)           (IN struct efi_simple_text_output_protocol *This, IN u64 ModeNumber);
    efi_status (EFI_API *SetAttribute)      (IN struct efi_simple_text_output_protocol *This, IN u64 Attribute);
    efi_status (EFI_API *ClearScreen)       (IN struct efi_simple_text_output_protocol *This);
    efi_status (EFI_API *SetCursorPosition) (IN struct efi_simple_text_output_protocol *This, IN u64 Column, IN u64 Row);
    efi_status (EFI_API *EnableCursor)      (IN struct efi_simple_text_output_protocol *This, IN b08 Visible);
    
    struct efi_simple_text_output_mode {
        s32 MaxMode;
        s32 Mode;
        s32 Attribute;
        s32 CursorColumn;
        s32 CursorRow;
        b08 CursorVisible;
    } *Mode;
} efi_simple_text_output_protocol;

typedef struct efi_device_path_protocol {
    u08 Type;
    u08 SubType;
    u08 Length[2];
} efi_device_path_protocol;

typedef struct efi_boot_services {
    efi_table_header Header;
    
    // Task Priority Services
    VER(EFI,1.0+)   efi_tpl    (EFI_API *RaiseTPL)   (IN efi_tpl NewTPL);
    VER(EFI,1.0+)   void       (EFI_API *RestoreTPL) (IN efi_tpl OldTPL);
    
    // Memory Services
    VER(EFI,1.0+)   efi_status (EFI_API *AllocatePages) (IN efi_allocate_type Type, IN efi_memory_type MemoryType, IN u64 Pages, IN OUT efi_physical_address *Memory);
    VER(EFI,1.0+)   efi_status (EFI_API *FreePages)     (IN efi_physical_address Memory, IN u64 Pages);
    VER(EFI,1.0+)   efi_status (EFI_API *GetMemoryMap)  (IN OUT u64 *MemoryMapSize, OUT efi_memory_descriptor *MemoryMap, OUT u64 *MapKey, OUT u64 *DescriptorSize, OUT u32 *DescriptorVersion);
    VER(EFI,1.0+)   efi_status (EFI_API *AllocatePool)  (IN efi_memory_type PoolType, IN u64 Size, OUT vptr *Buffer);
    VER(EFI,1.0+)   efi_status (EFI_API *FreePool)      (IN vptr Buffer);
    
    // Event & Timer Services
    VER(EFI,1.0+)   efi_status (EFI_API *CreateEvent)  (IN u32 Type, IN efi_tpl NotifyTPL, IN OPT func_EFI_EventNotify NotifyFunction, IN OPT vptr NotifyContext, OUT efi_event *Event);
    VER(EFI,1.0+)   efi_status (EFI_API *SetTimer)     (IN efi_event Event, IN efi_timer_delay Type, IN u64 TriggerTime);
    VER(EFI,1.0+)   efi_status (EFI_API *WaitForEvent) (IN u64 NumberOfEvents, IN efi_event *Event, OUT u64 *Index);
    VER(EFI,1.0+)   efi_status (EFI_API *SignalEvent)  (IN efi_event Event);
    VER(EFI,1.0+)   efi_status (EFI_API *CloseEvent)   (IN efi_event Event);
    VER(EFI,1.0+)   efi_status (EFI_API *CheckEvent)   (IN efi_event Event);
    
    // Protocol Handler Services
    VER(EFI,1.0+)   efi_status (EFI_API *InstallProtocolInterface)   (IN OUT efi_handle *Handle, IN efi_guid *Protocol, IN efi_interface_type InterfaceType, IN vptr Interface);
    VER(EFI,1.0+)   efi_status (EFI_API *ReinstallProtocolInterface) (IN efi_handle Handle, IN efi_guid *Protocol, IN vptr OldInterface, IN vptr NewInterface);
    VER(EFI,1.0+)   efi_status (EFI_API *UninstallProtocolInterface) (IN efi_handle Handle, IN efi_guid *Protocol, IN vptr Interface);
    VER(EFI,1.0+)   efi_status (EFI_API *HandleProtocol)             (IN efi_handle Handle, IN efi_guid *Protocol, OUT vptr *Interface);
    VER(EFI,1.0+)   vptr                 Reserved;
    VER(EFI,1.0+)   efi_status (EFI_API *RegisterProtocolNotify)     (IN efi_guid *Protocol, IN efi_event Event, OUT vptr *Registration);
    VER(EFI,1.0+)   efi_status (EFI_API *LocateHandle)               (IN efi_locate_search_type SearchType, IN OPT efi_guid *Protocol, IN OPT vptr SearchKey, IN OUT u64 *BufferSize, OUT efi_handle *Buffer);
    VER(EFI,1.0+)   efi_status (EFI_API *LocateDevicePath)           (IN efi_guid *Protocol, IN OUT efi_device_path_protocol **DevicePath, OUT efi_handle *Device);
    VER(EFI,1.0+)   efi_status (EFI_API *InstallConfigurationTable)  (IN efi_guid *Guid, IN vptr Table);
    
    // Image Services
    VER(EFI,1.0+)   efi_status (EFI_API *LoadImage)        (IN b08 BootPolicy, IN efi_handle ParentImageHandle, IN efi_device_path_protocol *DevicePath, IN OPT vptr SourceBuffer, IN u64 SourceSize, OUT efi_handle *ImageHandle);
    VER(EFI,1.0+)   efi_status (EFI_API *StartImage)       (IN efi_handle ImageHandle, OUT u64 *ExitDataSize, OUT OPT c16 **ExitData);
    VER(EFI,1.0+)   efi_status (EFI_API *Exit)             (IN efi_handle ImageHandle, IN efi_status ExitStatus, IN u64 ExitDataSize, IN OPT c16 *ExitData);
    VER(EFI,1.0+)   efi_status (EFI_API *UnloadImage)      (IN efi_handle ImageHandle);
    VER(EFI,1.0+)   efi_status (EFI_API *ExitBootServices) (IN efi_handle ImageHandle, IN u64 MapKey);
    
    // Miscellaneous Services
    VER(EFI,1.0+)   efi_status (EFI_API *GetNextMonotonicCount) (OUT u64 *Count);
    VER(EFI,1.0+)   efi_status (EFI_API *Stall)                 (IN u64 Microseconds);
    VER(EFI,1.0+)   efi_status (EFI_API *SetWatchdogTimer)      (IN u64 Timeout, IN u64 WatchdogCode, IN u64 DataSize, IN OPT c16 *WatchdogData);
    
    // DriverSupport Services
    VER(EFI,1.1)    efi_status (EFI_API *ConnectController)    (IN efi_handle ControllerHandle, IN OPT efi_handle *DriverImageHandle, IN OPT efi_device_path_protocol *RemainingDevicePath, IN b08 Recursive);
    VER(EFI,1.1+)   efi_status (EFI_API *DisconnectController) (IN efi_handle ControllerHandle, IN OPT efi_handle DriverImageHandle, IN OPT efi_handle ChildHandle);
    
    // Open and Close Protocol Services
    VER(EFI,1.1+)   efi_status (EFI_API *OpenProtocol)            (IN efi_handle Handle, IN efi_guid *Protocol, OUT OPT vptr *Interface, IN efi_handle AgentHandle, IN efi_handle ControllerHandle, IN u32 Attributes);
    VER(EFI,1.1+)   efi_status (EFI_API *CloseProtocol)           (IN efi_handle Handle, IN efi_guid *Protocol, IN efi_handle AgentHandle, IN efi_handle ControllerHandle);
    VER(EFI,1.1+)   efi_status (EFI_API *OpenProtocolInformation) (IN efi_handle Handle, IN efi_guid *Protocol, OUT efi_open_protocol_information_entry *EntryBuffer, OUT u64 *EntryCount);
    
    // Library Services
    VER(EFI,1.1+)   efi_status (EFI_API *ProtocolsPerHandle)                  (IN efi_handle Handle, OUT efi_guid ***ProtocolBuffer, OUT u64 *ProtocolBufferCount);
    VER(EFI,1.1+)   efi_status (EFI_API *LocateHandleBuffer)                  (IN efi_locate_search_type SearchType, IN OPT efi_guid *Protocol, IN OPT vptr SearchKey, OUT u64 *NoHandles, OUT efi_handle **Buffer);
    VER(EFI,1.1+)   efi_status (EFI_API *LocateProtocol)                      (IN efi_guid *Protocol, IN OPT vptr Registration, OUT vptr *Interface);
    VER(EFI,1.1+)   efi_status (EFI_API *InstallMultipleProtocolInterfaces)   (IN OUT efi_handle *Handle, ...);
    VER(EFI,1.1+)   efi_status (EFI_API *UninstallMultipleProtocolInterfaces) (IN efi_handle Handle, ...);
    
    // 32-bit CRC Services
    VER(EFI,1.1+)   efi_status (EFI_API *CalculateCrc32) (IN vptr Data, IN u64 DataSize, OUT u32 *Crc32);
    
    // Miscellaneous Services
    VER(EFI,1.1+)   void       (EFI_API *CopyMem)       (IN vptr Destination, IN vptr Source, IN u64 Length);
    VER(EFI,1.1+)   void       (EFI_API *SetMem)        (IN vptr Buffer, IN u64 Size, IN u08 Value);
    VER(UEFI,2.0+)  efi_status (EFI_API *CreateEventEx) (IN u32 Type, IN efi_tpl NotifyTPL, IN OPT func_EFI_EventNotify NotifyFunction, IN OPT CONST vptr NotifyContext, IN OPT CONST efi_guid *EventGroup, OUT efi_event *Event);
} efi_boot_services;

typedef struct efi_runtime_services {
    efi_table_header Header;
    
    // Time Services
    efi_status (EFI_API *GetTime)       (OUT efi_time *Time, OUT OPT efi_time_capabilities *Capabilities);
    efi_status (EFI_API *SetTime)       (IN efi_time *Time);
    efi_status (EFI_API *GetWakeupTime) (OUT b08 *Enabled, OUT b08 *Pending, OUT efi_time *Time);
    efi_status (EFI_API *SetWakeupTime) (IN b08 Enable, IN OPT efi_time *Time);
    
    // Virtual Memory Services
    efi_status (EFI_API *SetVirtualAddressMap) (IN u64 MemoryMapSize, IN u64 DescriptorSize, IN u32 DescriptorVersion, IN efi_memory_descriptor *VirtualMap);
    efi_status (EFI_API *ConvertPointer)       (IN u64 DebugDisposition, IN vptr *Address);
    
    // Variable Services
    efi_status (EFI_API *GetVariable)         (IN c16 *VariableName, IN efi_guid *VendorGuid, OUT OPT u32 *Attributes, IN OUT u64 *DataSize, OUT OPT vptr Data);
    efi_status (EFI_API *GetNextVariableName) (IN OUT u64 *VariableNameSize, IN OUT c16 *VariableName, IN OUT efi_guid *VendorGuid);
    efi_status (EFI_API *SetVariable)         (IN c16 *VariableName, IN efi_guid *VendorGuid, IN u32 Attributes, IN u64 DataSize, IN vptr Data);
    
    // Miscellaneous Services
    efi_status (EFI_API *GetNextHighMonotonicCount) (OUT u32 *HighCount);
    void       (EFI_API *ResetSystem)               (IN efi_reset_type ResetType, IN efi_status ResetStatus, IN u64 DataSize, IN OPT vptr ResetData);
    
    // UEFI 2.0 Capsule Services
    efi_status (EFI_API *UpdateCapsule)            (IN efi_capsule_header **CapsuleHeaderArray, IN u64 CapsuleCount, IN OPT efi_physical_address ScatterGatherList);
    efi_status (EFI_API *QueryCapsuleCapabilities) (IN efi_capsule_header **CapsuleHeaderArray, IN u64 CapsuleCount, OUT u64 *MaximumCapsuleSize, OUT efi_reset_type *ResetType);
    
    // Miscellaneous UEFI 2.0 Service
    efi_status (EFI_API *QueryVariableInfo) (IN u32 Attributes, OUT u64 *MaximumVariableStorageSize, OUT u64 *RemainingVariableStorageSize, OUT u64 *MaximumVariableSize);
} efi_runtime_services;

typedef struct efi_configuration_table {
    efi_guid VendorGuid;
    vptr VendorTable;
} efi_configuration_table;

typedef struct efi_system_table {
    efi_table_header Header;
    c16 *FirmwareVendor;
    u32 FirmwareRevision;
    efi_handle ConsoleInHandle;
    efi_simple_text_input_protocol *ConsoleIn;
    efi_handle ConsoleOutHandle;
    efi_simple_text_output_protocol *ConsoleOut;
    efi_handle StandardErrorHandle;
    efi_simple_text_output_protocol *StandardError;
    efi_runtime_services *RuntimeServices;
    efi_boot_services *BootServices;
    u64 NumberOfTableEntries;
    efi_configuration_table *ConfigurationTable;
} efi_system_table;