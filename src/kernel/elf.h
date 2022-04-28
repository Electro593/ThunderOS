typedef u64 elf64_xword;
typedef s64 elf64_sxword;
typedef u64 elf64_addr;

#define DT_NULL             0 // Marks end of dynamic section
#define DT_RELA             7 // Address of Rela relocs
#define DT_RELASZ           8 // Total size of Rela relocs
#define DT_RELAENT          9 // Size of one Rela reloc
typedef struct elf64_dynamic {
    elf64_sxword Tag;
    union {
        elf64_xword Value;  // Integer value
        elf64_addr Address; // Virtual address
    };
} elf64_dynamic;

#define ELF64_R_TYPE(Info) ((Info) & 0xFFFFFFFF)
typedef struct elf64_relocation {
    elf64_addr Offset; // Base address to be relocated to
    elf64_xword Info;  // Relocation type and symbol index
} elf64_relocation;

// Types for ELF64_R_TYPE(Info)
#define R_X86_64_RELATIVE   8 // Adjust by program base

enum elf_program_header_type {
    ELF_ProgramHeaderType_Null,
    ELF_ProgramHeaderType_Load,
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
	u16	SectionNamesSection;
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