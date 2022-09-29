/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                         **
**  Author: Aria Seiler                                                    **
**                                                                         **
**  This program is in the public domain. There is no implied warranty,    **
**  so use it at your own risk.                                            **
**                                                                         **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

enum elf_header_type {
    ELF_HeaderType_None = 0,
    ELF_HeaderType_Rel  = 1,
    ELF_HeaderType_Exec = 2,
    ELF_HeaderType_Dyn  = 3,
    ELF_HeaderType_Code = 4
};

enum elf_machine_type {
    ELF_MachineType_None        = 0x00,
    ELF_MachineType_Sparc       = 0x02,
    ELF_MachineType_x86         = 0x03,
    ELF_MachineType_Sparc32Plus = 0x12,
    ELF_MachineType_ARM         = 0x28,
    ELF_MachineType_SparcV9     = 0x2B,
    ELF_MachineType_IA64        = 0x32,
    ELF_MachineType_AMD64       = 0x3E,
    ELF_MachineType_ARM64       = 0xB7,
    ELF_MachineType_RISCV       = 0xF3,
};

enum elf_program_header_type {
    ELF_ProgramHeaderType_Null          = 0x00000000,
    ELF_ProgramHeaderType_Load          = 0x00000001,
    ELF_ProgramHeaderType_Dynamic       = 0x00000002,
    ELF_ProgramHeaderType_Interp        = 0x00000003,
    ELF_ProgramHeaderType_Note          = 0x00000004,
    ELF_ProgramHeaderType_SHLIB         = 0x00000005,
    ELF_ProgramHeaderType_ProgramHeader = 0x00000006,
    ELF_ProgramHeaderType_TLS           = 0x00000007,
    
    ELF_ProgramHeaderType_GNUStack      = 0x6474E551,
};

enum elf_section_header_type {
    ELF_SectionHeaderType_Null               = 0x00,
    ELF_SectionHeaderType_ProgramBits        = 0x01,
    ELF_SectionHeaderType_SymbolTable        = 0x02,
    ELF_SectionHeaderType_StringTable        = 0x03,
    ELF_SectionHeaderType_RelocAddends       = 0x04,
    ELF_SectionHeaderType_Hash               = 0x05,
    ELF_SectionHeaderType_Dynamic            = 0x06,
    ELF_SectionHeaderType_Note               = 0x07,
    ELF_SectionHeaderType_NoBits             = 0x08,
    ELF_SectionHeaderType_Reloc              = 0x09,
    ELF_SectionHeaderType_SHLIB              = 0x0A,
    ELF_SectionHeaderType_DynamicSymbolTable = 0x0B,
    ELF_SectionHeaderType_InitArray          = 0x0E,
    ELF_SectionHeaderType_FiniArray          = 0x0F,
    ELF_SectionHeaderType_PreinitArray       = 0x10,
    ELF_SectionHeaderType_Group              = 0x11,
    ELF_SectionHeaderType_SymbolTableEX      = 0x12,
};

enum elf_section_header_flags {
    ELF_SectionHeaderFlag_Write         = 0x001,
    ELF_SectionHeaderFlag_Alloc         = 0x002,
    ELF_SectionHeaderFlag_Exec          = 0x004,
    ELF_SectionHeaderFlag_Merge         = 0x010,
    ELF_SectionHeaderFlag_Strings       = 0x020,
    ELF_SectionHeaderFlag_InfoLink      = 0x040,
    ELF_SectionHeaderFlag_LinkOrder     = 0x080,
    ELF_SectionHeaderFlag_NonConforming = 0x100,
    ELF_SectionHeaderFlag_Group         = 0x200,
    ELF_SectionHeaderFlag_TLS           = 0x400,
};

enum elf_symbol_binding {
    ELF_SymbolBinding_Local  =  0,
    ELF_SymbolBinding_Global =  1,
    ELF_SymbolBinding_Weak   =  2,
};

enum elf_symbol_type {
    ELF_SymbolType_None     = 0,
    ELF_SymbolType_Object   = 1,
    ELF_SymbolType_Function = 2,
    ELF_SymbolType_Section  = 3,
    ELF_SymbolType_File     = 4,
};

enum elf_relocation_type {
    ELF_RelocationType_AMD64_None     = 0x00,
    ELF_RelocationType_AMD64_64       = 0x01,
    ELF_RelocationType_AMD64_PC32     = 0x02,
    ELF_RelocationType_AMD64_GOT32    = 0x03,
    ELF_RelocationType_AMD64_PLT32    = 0x04,
    ELF_RelocationType_AMD64_Copy     = 0x05,
    ELF_RelocationType_AMD64_Global   = 0x06,
    ELF_RelocationType_AMD64_Jump     = 0x07,
    ELF_RelocationType_AMD64_Rel      = 0x08,
    ELF_RelocationType_AMD64_GOTPCRel = 0x09,
    ELF_RelocationType_AMD64_32       = 0x0A,
    ELF_RelocationType_AMD64_32S      = 0x0B,
    ELF_RelocationType_AMD64_16       = 0x0C,
    ELF_RelocationType_AMD64_PC16     = 0x0D,
    ELF_RelocationType_AMD64_8        = 0x0E,
    ELF_RelocationType_AMD64_PC8      = 0x0F,
    ELF_RelocationType_AMD64_DPTMod64 = 0x10,
    ELF_RelocationType_AMD64_DTPOff64 = 0x11,
    ELF_RelocationType_AMD64_TPOff64  = 0x12,
    ELF_RelocationType_AMD64_TLSGD    = 0x13,
    ELF_RelocationType_AMD64_TLSLD    = 0x14,
    ELF_RelocationType_AMD64_DTPOff32 = 0x15,
    ELF_RelocationType_AMD64_GOTTPOff = 0x16,
    ELF_RelocationType_AMD64_TPOff32  = 0x17,
    ELF_RelocationType_AMD64_GOTPCRelX = 0x2A,
};

typedef struct elf64_header {
    u08 ID[16];
    u16 Type;
    u16 Machine;
    u32 Version;
    u64 Entry;
    u64 ProgramHeaderOffset;
    u64 SectionHeaderOffset;
	u32	Flags;
	u16	ELFHeaderSize;
	u16	ProgramHeaderSize;
	u16	ProgramHeaderCount;
	u16	SectionHeaderSize;
	u16	SectionHeaderCount;
	u16	StringTableIndex;
} elf64_header;

typedef struct elf64_program_header {
    u32 Type;
    u32 Flags;
    u64 Offset;
    u64 VirtualAddress;
    u64 PhysicalAddress;
    u64 SizeInFile;
    u64 SizeInMemory;
    u64 Align;
} elf64_program_header;

typedef struct elf64_section_header {
    u32 Name;
    u32 Type;
    u64 Flags;
    u64 VirtualAddress;
    u64 Offset;
    u64 Size;
    u32 Link;
    u32 Info;
    u64 Align;
    u64 EntrySize;
} elf64_section_header;

typedef struct elf64_symbol {
    u32 Name;
    u08 Info;
    u08 Other;
    u16 SectionIndex;
    u64 Value;
    u64 Size;
} elf64_symbol;

typedef struct elf64_relocation {
    u64 Offset;
    u64 Info;
} elf64_relocation;

typedef struct elf64_relocation_addend {
    u64 Offset;
    u64 Info;
    s64 Addend;
} elf64_relocation_addend;

typedef struct elf64_dynamic {
    s64 Tag;
    union {
        u64 Value;
        u64 Address;
    };
} elf64_dynamic;