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