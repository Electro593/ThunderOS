ELF_Header :: struct {
   MAGIC_NUMBER :: u8.[0x7F, #char "E", #char "L", #char "F"];
   magic_number: [4]u8 = MAGIC_NUMBER;
   
   word_size: enum u8 {
      IS_32_BITS :: 1;
      IS_64_BITS :: 2;
   }
   
   endianness: enum u8 {
      LITTLE :: 1;
      BIG    :: 2;
   }
   
   elf_header_version: u8 = 1;
   
   os_abi: enum u8 {
      SYSTEM_V                     :: 0x00;
      HP_UX                        :: 0x01;
      NETBSD                       :: 0x02;
      LINUX                        :: 0x03;
      GNU_HURD                     :: 0x04;
      SOLARIS                      :: 0x06;
      AIX_MONTEREY                 :: 0x07;
      IRIX                         :: 0x08;
      FREEBSD                      :: 0x09;
      TRU64                        :: 0x0A;
      NOVELL_MODESTO               :: 0x0B;
      OPENBSD                      :: 0x0C;
      OPENVMS                      :: 0x0D;
      NONSTOP_KERNEL               :: 0x0E;
      AROS                         :: 0x0F;
      FENIXOS                      :: 0x10;
      NUXI_CLOUDABI                :: 0x11;
      STRATUS_TECHNOLOGIES_OPENVOS :: 0x12;
   }
   
   abi_version: u8;
   
   _reserved: [7]u8;
   
   type: enum u16 {
      NONE        :: 0x0;
      RELOCATABLE :: 0x1;
      EXECUTABLE  :: 0x2;
      DYNAMIC     :: 0x3;
      CORE        :: 0x4;
   }
   
   instruction_set: enum u16 {
      UNSPECIFIED                   :: 0x000;
      WE_32100                      :: 0x001;
      SPARC                         :: 0x002;
      X86                           :: 0x003;
      MOTOROLA_68000                :: 0x004;
      MOTOROLA_88000                :: 0x005;
      INTEL_MCU                     :: 0x006;
      INTEL_80860                   :: 0x007;
      MIPS                          :: 0x008;
      IBM_SYSTEM_370                :: 0x009;
      MIPS_RS3000                   :: 0x00A;
      HP_PA_RISC                    :: 0x00F;
      INTEL_80960                   :: 0x013;
      POWERPC_32                    :: 0x014;
      POWERPC_64                    :: 0x015;
      S390                          :: 0x016;
      IBM_SPU_SPC                   :: 0x017;
      NEC_V800                      :: 0x024;
      FUJITSU_FR20                  :: 0x025;
      TRW_RH_32                     :: 0x026;
      MOTOROLA_RCE                  :: 0x027;
      ARM_32                        :: 0x028;
      DIGITAL_ALPHA                 :: 0x029;
      SUPERH                        :: 0x02A;
      SPARC_9                       :: 0x02B;
      SIEMENS_TRICORE               :: 0x02C;
      ARGONAUT_RISC_CORE            :: 0x02D;
      HITACHI_H8_300                :: 0x02E;
      HITACHI_H8_300H               :: 0x02F;
      HITACHI_H8S                   :: 0x030;
      HITACHI_H8_500                :: 0x031;
      IA_64                         :: 0x032;
      STANFORD_MIPS_X               :: 0x033;
      MOTOROLA_COLDFIRE             :: 0x034;
      MOTOROLA_M68HC12              :: 0x035;
      FUJITSU_MMA                   :: 0x036;
      SIEMENS_PCP                   :: 0x037;
      SONY_NCPU_RISC                :: 0x038;
      DENSO_NDR1                    :: 0x039;
      MOTOROLA_STAR_CORE            :: 0x03A;
      TOYOTA_ME16                   :: 0x03B;
      STMICROELECTRONICS_ST100      :: 0x03C;
      ADVANCED_LOGIC_CORP_TINYJ     :: 0x03D;
      AMD_X86_64                    :: 0x03E;
      SONY_DSP                      :: 0x03F;
      DIGITAL_EQUIPMENT_CORP_PDP_10 :: 0x040;
      DIGITAL_EQUIPMENT_CORP_PDP_11 :: 0x041;
      SIEMENS_FX66                  :: 0x042;
      STMICROELECTRONICS_ST9_PLUS   :: 0x043;
      STMICROELECTRONICS_ST7        :: 0x044;
      MOTOROLA_MC68HC16             :: 0x045;
      MOTOROLA_MC68HC11             :: 0x046;
      MOTOROLA_MC68HC08             :: 0x047;
      MOTOROLA_MC68HC05             :: 0x048;
      SILICON_GRAPHICS_SVX          :: 0x049;
      STMICROELECTRONICS_ST19       :: 0x04A;
      DIGITAL_VAX                   :: 0x04B;
      AXIS_COMMUNICATIONS_32        :: 0x04C;
      INFINEON_TECHNOLOGIES_32      :: 0x04D;
      ELEMENT_14_64_DSP             :: 0x04E;
      LSI_LOGIC_16_DSP              :: 0x04F;
      TMS320C6000                   :: 0x08C;
      MCST_ELBRUS_E2K               :: 0x0AF;
      ARM_64                        :: 0x0B7;
      ZILOG_Z80                     :: 0x0DC;
      RISC_V                        :: 0x0F3;
      BERKELEY_PACKET_FILTER        :: 0x0F7;
      WDC_65C816                    :: 0x101;
   }
   elf_version: u32 = 1;
}

ELF_Header_64 :: struct {
   #as using _: ELF_Header;
   word_size = .IS_64_BITS;
   
   entry_point_address: u64;
   program_header_offset: u64;
   section_header_offset: u64;
   
   flags: u32;
   
   elf_header_size: u16;
   
   program_header_entry_size: u16;
   program_header_entry_count: u16;
   
   section_header_entry_size: u16;
   section_header_entry_count: u16;
   
   string_table_index: u16;
}

ELF_Header_32 :: struct {
   #as using _: ELF_Header;
   word_size = .IS_32_BITS;
   
   entry_point_address: u32;
   program_header_offset: u32;
   section_header_offset: u32;
   
   flags: u32;
   
   elf_header_size: u16;
   
   program_header_entry_size: u16;
   program_header_entry_count: u16;
   
   section_header_entry_size: u16;
   section_header_entry_count: u16;
   
   string_table_index: u16;
}

ELF_Program_Header_Type :: enum u32 {
   NULL             :: 0x0;
   LOADABLE         :: 0x1;
   DYNAMIC          :: 0x2;
   INTERPRETER_INFO :: 0x3;
   NOTE             :: 0x4;
   PROGRAM_HEADER   :: 0x6;
   TLS_TEMPLATE     :: 0x7;
}

ELF_Program_Header_Flags :: enum_flags u32 {
   EXECUTABLE :: 1 << 0;
   WRITABLE   :: 1 << 1;
   READABLE   :: 1 << 2;
}

ELF_Program_Header_64 :: struct {
   type: ELF_Program_Header_Type;
   
   flags: ELF_Program_Header_Flags;
   
   offset_in_file: u64;
   virtual_address: u64;
   physical_address: u64;
   
   size_in_file: u64;
   size_in_memory: u64;
   
   alignment: u64;
}

ELF_Program_Header_32 :: struct {
   type: ELF_Program_Header_Type;
   
   offset_in_file: u32;
   virtual_address: u32;
   physical_address: u32;
   
   size_in_file: u32;
   size_in_memory: u32;
   
   flags: ELF_Program_Header_Flags;
   
   alignment: u32;
}

ELF_Section_Header_Type :: enum u32 {
   UNUSED                   :: 0x00;
   PROGRAM_DATA             :: 0x01;
   SYMBOL_TABLE             :: 0x02;
   STRING_TABLE             :: 0x03;
   RELOCATIONS_WITH_ADDENDS :: 0x04;
   SYMBOL_HASH              :: 0x05;
   DYNAMIC_INFO             :: 0x06;
   NOTE                     :: 0x07;
   BSS_DATA                 :: 0x08;
   RELOCATIONS              :: 0x09;
   DYNAMIC_SYMBOL_TABLE     :: 0x0B;
   CONSTRUCTORS             :: 0x0E;
   DESTRUCTORS              :: 0x0F;
   PRE_CONSTRUCTORS         :: 0x10;
   GROUP                    :: 0x11;
   EXTENDED_SECTION_INDICES :: 0x12;
}

ELF_Section_Header_64 :: struct {
   name_offset: u32;
   
   type: ELF_Section_Header_Type;
   
   flags: enum_flags u64 {
      WRITABLE         :: 1 << 0;
      IN_MEMORY        :: 1 << 1;
      EXECUTABLE       :: 1 << 2;
      MAYBE_MERGED     :: 1 << 4;
      HAS_STRINGS      :: 1 << 5;
      INDEX_IN_INFO    :: 1 << 6;
      PRESERVE_ORDER   :: 1 << 7;
      OS_NONCONFORMING :: 1 << 8;
      IS_IN_GROUP      :: 1 << 9;
      HAS_TLS          :: 1 << 10;
      ORDERED          :: 1 << 26;
      EXCLUDE          :: 1 << 27;
   }
   
   virtual_address: u64;
   offset_in_file: u64;
   size_in_file: u64;
   
   linked_section_index: u32;
   info: u32;
   
   alignment: u64;
   entry_size: u64;
}

ELF_Section_Header_32 :: struct {
   name_offset: u32;
   
   type: ELF_Section_Header_Type;
   
   flags: enum_flags u32 {
      WRITABLE         :: 1 << 0;
      IN_MEMORY        :: 1 << 1;
      EXECUTABLE       :: 1 << 2;
      MAYBE_MERGED     :: 1 << 4;
      HAS_STRINGS      :: 1 << 5;
      INDEX_IN_INFO    :: 1 << 6;
      PRESERVE_ORDER   :: 1 << 7;
      OS_NONCONFORMING :: 1 << 8;
      IS_IN_GROUP      :: 1 << 9;
      HAS_TLS          :: 1 << 10;
      ORDERED          :: 1 << 26;
      EXCLUDE          :: 1 << 27;
   }
   
   virtual_address: u32;
   offset_in_file: u32;
   size_in_file: u32;
   
   linked_section_index: u32;
   info: u32;
   
   alignment: u32;
   entry_size: u32;
}

load_elf :: (memory: *void, memory_size: int, file_data: *u8) -> entry: *void, success: bool {
   
}