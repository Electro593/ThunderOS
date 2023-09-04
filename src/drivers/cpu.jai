CR0 :: enum_flags u64 {
   PROTECTION_ENABLE   :: 1 <<  0;
   MONITOR_COPROCESSOR :: 1 <<  1;
   EMULATION           :: 1 <<  2;
   TASK_SWITCHED       :: 1 <<  3;
   EXTENSION_TYPE      :: 1 <<  4;
   NUMERIC_ERROR       :: 1 <<  5;
   WRITE_PROTECT       :: 1 << 16;
   ALIGNMENT_MASKED    :: 1 << 18;
   NOT_WRITE_THROUGH   :: 1 << 29;
   CACHE_DISABLE       :: 1 << 30;
   PAGING              :: 1 << 31;
}

CR2 :: enum_flags u64 {
   PAGE_FAULT_LINEAR_ADDRESS_MASK :: 0xFFFF_FFFF_FFFF_FFFF;
}

CR3 :: enum_flags u64 {
   WRITE_THROUGH :: 1 << 3;
   CACHE_DISABLE :: 1 << 4;
   
   ADDRESS_MASK :: 0xFFFF_FFFF_FFFF_F000;
}

CR4 :: enum_flags u64 {
   VIRTUAL_8086_MODE_EXTENSIONS          :: 1 <<  0;
   PROTECTED_MODE_VIRTUAL_INTERRUPTS     :: 1 <<  1;
   TIME_STAMP_DISABLE                    :: 1 <<  2;
   DEBUGGING_EXTENSIONS                  :: 1 <<  3;
   PAGE_SIZE_EXTENSIONS                  :: 1 <<  4;
   PHYSICAL_ADDRESS_EXTENSION            :: 1 <<  5;
   MACHINE_CHECK_ENABLE                  :: 1 <<  6;
   PAGE_GLOBAL_ENABLE                    :: 1 <<  7;
   PERFORMANCE_MONITORING_COUNTER_ENABLE :: 1 <<  8;
   OS_SUPPORT_FOR_FXSAVE_AND_FXRSTOR     :: 1 <<  9;
   OS_SUPPORT_FOR_SIMD_EXCEPTIONS        :: 1 << 10;
   USER_MODE_INSTRUCTION_PREVENTATION    :: 1 << 11;
   LINEAR_ADDRESS_57                     :: 1 << 12;
   VMX_ENABLE                            :: 1 << 13;
   SMX_ENABLE                            :: 1 << 14;
   FSGSBASE_ENABLE                       :: 1 << 16;
   PCID_ENABLE                           :: 1 << 17;
   XSAVE_AND_EXTENDED_STATES_ENABLE      :: 1 << 18;
   KEY_LOCKER_ENABLE                     :: 1 << 19;
   SMEP_ENABLE                           :: 1 << 20;
   SMAP_ENABLE                           :: 1 << 21;
   USER_PROTECTION_KEYS                  :: 1 << 22;
   CONTROL_FLOW_ENFORCEMENT              :: 1 << 23;
   SUPERVISOR_PROTECTION_KEYS            :: 1 << 24;
}