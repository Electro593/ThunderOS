ELF :: struct {
   header: *ELF_Header;
   
   program_headers: Sparse_Array(ELF_Program_Header);
   section_headers: Sparse_Array(ELF_Section_Header);
   
   section_header_string_table: [] u8;
   
   data: *u8;
}

ELF_Hash :: struct {
   buckets: [] u32;
   chains: [] u32;
   
   symbols: Sparse_Array(ELF_Symbol);
   strings: *u8;
}

ELF_Header :: struct {
   MAGIC_NUMBER :: u8.[0x7F, #char "E", #char "L", #char "F"];
   magic_number: [4]u8 = MAGIC_NUMBER;
   
   word_size: enum u8 {
      IS_32_BITS :: 1;
      IS_64_BITS :: 2;
   } = .IS_64_BITS;
   
   endianness: enum u8 {
      LITTLE :: 1;
      BIG    :: 2;
   } = .LITTLE;
   
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
   } = .SYSTEM_V;
   
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
      FUJITSU_VPP500                :: 0x011;
      SPARC_32_PLUS                 :: 0x012;
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
      MMIX                          :: 0x050;
      HUANY                         :: 0x051;
      PRISM                         :: 0x052;
      AVR                           :: 0x053;
      FR30                          :: 0x054;
      D10V                          :: 0x055;
      D30V                          :: 0x056;
      V850                          :: 0x057;
      M32R                          :: 0x058;
      MN10300                       :: 0x059;
      MN10200                       :: 0x05A;
      PJ                            :: 0x05B;
      OPENRISC                      :: 0x05C;
      ARC_A5                        :: 0x05D;
      XTENSA                        :: 0x05E;
      VIDEOCORE                     :: 0x05F;
      TMM_GPP                       :: 0x060;
      NS32K                         :: 0x061;
      TPC                           :: 0x062;
      SNP1K                         :: 0x063;
      ST200                         :: 0x064;
      TMS320C6000                   :: 0x08C;
      MCST_ELBRUS_E2K               :: 0x0AF;
      ARM_64                        :: 0x0B7;
      ZILOG_Z80                     :: 0x0DC;
      RISC_V                        :: 0x0F3;
      BERKELEY_PACKET_FILTER        :: 0x0F7;
      WDC_65C816                    :: 0x101;
   }
   
   elf_version: u32 = 1;
   
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

ELF_Program_Header :: struct {
   type: enum u32 {
      NULL             :: 0x0;
      LOADABLE         :: 0x1;
      DYNAMIC          :: 0x2;
      INTERPRETER_INFO :: 0x3;
      NOTE             :: 0x4;
      PROGRAM_HEADER   :: 0x6;
      TLS_TEMPLATE     :: 0x7;
      
      #if CPU == .X64 {
         EH_FRAME :: 0x6474E550;
         STACK    :: 0x6474E551;
         RELRO    :: 0x6474E552;
      }
   }
   
   flags: enum_flags u32 {
      EXECUTABLE :: 1 << 0;
      WRITABLE   :: 1 << 1;
      READABLE   :: 1 << 2;
   }
   
   offset_in_file: u64;
   virtual_address: u64;
   physical_address: u64;
   
   size_in_file: u64;
   size_in_memory: u64;
   
   alignment: u64;
}

ELF_Section_Header :: struct {
   name_offset: u32;
   
   type: enum u32 {
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
      
      GNU_HASH                 :: 0x6FFFFFF6;
      GNU_VERSION_REQUIREMENTS :: 0x6FFFFFFE;
      GNU_VERSION              :: 0x6FFFFFFF;
   }
   
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

ELF_Symbol :: struct {
   name_offset: s32;
   
   info: enum u8 {
      // Type
      NO_TYPE  :: 0;
      OBJECT   :: 1;
      FUNCTION :: 2;
      SECTION  :: 3;
      FILE     :: 4;
      COMMON   :: 5;
      TLS      :: 6;
      
      // Binding
      LOCAL  :: 0 << 4;
      GLOBAL :: 1 << 4;
      WEAK   :: 2 << 4;
   }
   
   other: enum u8 {
      // Visibility
      DEFAULT   :: 0;
      INTERNAL  :: 1;
      HIDDEN    :: 2;
      PROTECTED :: 3;
   }
   
   section_index: enum s16 {
      UNDEFINED :: 0x0000;
      BEFORE    :: 0xFF00;
      AFTER     :: 0xFF01;
      ABSOLUTE  :: 0xFFF1;
      COMMON    :: 0xFFF2;
      TOO_LARGE :: 0xFFFF;
   }
   
   value: u64;
   size: u64;
}

ELF_Relocation :: struct {
   offset: u64;
   
   type: ELF_Relocation_Type;
   symbol: u32;
   
   #place type;
   info: u64 = ---;
}

ELF_Relocation_With_Addend :: struct {
   offset: u64;
   
   type: ELF_Relocation_Type;
   symbol_index: u32;
   
   addend: s64;
   
   #place type;
   info: u64 = ---;
}

ELF_Relocation_Type :: enum u32 {
   #if CPU == .X64 {
      //
      // A = Addend
      // B = Base address
      // G = GOT Offset
      // GOT = Address of GOT
      // L = PLT Offset
      // P = Offset
      // S = Symbol value
      //
      NONE      :: 0;  // none
      A64       :: 1;  // cast(u64) (S + A)
      PC32      :: 2;  // cast(u32) (S + A - P)
      GOT32     :: 3;  // cast(u32) (G + A)
      PLT32     :: 4;  // cast(u32) (L + A - P)
      COPY      :: 5;  // none
      GLOB_DAT  :: 6;  // cast(u64) (S)
      JUMP_SLOT :: 7;  // cast(u64) (S)
      RELATIVE  :: 8;  // cast(u64) (B + A)
      GOTPCREL  :: 9;  // cast(u32) (G + GOT + A - P)
      A32       :: 10; // cast(u32) (S + A)
      A32S      :: 11; // cast(s32) (S + A)
   // A16       :: 12; // cast(u16) (S + A)
   // PC16      :: 13; // cast(u16) (S + A - P)
   // A8        :: 14; // cast(u8)  (S + A)
   // PC8       :: 15; // cast(u8)  (S + A - P)
      DPTMOD64  :: 16; //
      DTPOFF64  :: 17; //
      TPOFF64   :: 18; //
      TLSGD     :: 19; //
      TLSLD     :: 20; //
      DTPOFF32  :: 21; //
      GOTTPOFF  :: 22; //
      TPOFF32   :: 23; //
      PC64      :: 24; // cast(u64) (S + A - P)
      GOTOFF64  :: 25; // cast(u64) (S + A - GOT)
      GOTPC32   :: 26; // cast(u32) (GOT + A - P)
   }
}

ELF_Dynamic :: struct {
   tag: enum s64 {
      NULL             :: 0;
      NEEDED           :: 1;
      PLTRELSZ         :: 2;
      PLTGOT           :: 3;
      HASH             :: 4;
      STRTAB           :: 5;
      SYMTAB           :: 6;
      RELA             :: 7;
      RELASZ           :: 8;
      RELAENT          :: 9;
      STRSZ            :: 10;
      SYMENT           :: 11;
      INIT             :: 12;
      FINI             :: 13;
      SONAME           :: 14;
      RPATH            :: 15;
      SYMBOLIC         :: 16;
      REL              :: 17;
      RELSZ            :: 18;
      RELENT           :: 19;
      PLTREL           :: 20;
      DEBUG            :: 21;
      TEXTREL          :: 22;
      JMPREL           :: 23;
      BIND_NOW         :: 24;
      INIT_ARRAY       :: 25;
      FINI_ARRAY       :: 26;
      INIT_ARRAYSZ     :: 27;
      FINI_ARRAYSZ     :: 28;
      RUNPATH          :: 29;
      FLAGS            :: 30;
      ENCODING         :: 32;
      PREINIT_ARRAY    :: 32;
      PREINIT_ARRAY_SZ :: 33;
   }
   value: u64;
}

ELF_Note :: struct {
   name_size: u64;
   descriptor_size: u64;
   
   type: s64;
   
   // name: ..u64;
   // descriptor: ..u64;
}

get_section_header :: inline (elf: ELF, index: int) -> *ELF_Section_Header {
   if index >= elf.section_headers.count return null;
   return elf.section_headers.data + index * elf.header.section_header_entry_size;
}

get_program_header :: inline (elf: ELF, index: int) -> *ELF_Program_Header {
   if index >= elf.program_headers.count return null;
   return elf.program_headers.data + index * elf.header.program_header_entry_size;
}

#scope_file

#import "Sparse_Array";