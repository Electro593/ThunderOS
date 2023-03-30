#load "../drivers/mem.jai";

CR0_Flags :: enum_flags u32 {
   PE :: 1 << 0;
   MP :: 1 << 1;
   EM :: 1 << 2;
   TS :: 1 << 3;
   ET :: 1 << 4;
   NE :: 1 << 4;
   WP :: 1 << 16;
   AM :: 1 << 18;
   NW :: 1 << 29;
   CD :: 1 << 30;
   PG :: 1 << 31;
}

CR1_Flags :: enum_flags u64 {
   
}

CR2_Flags :: enum_flags u64 {
   PFLA :: 0xFFFF_FFFF_FFFF_FFFF;
}

CF3_Flags :: enum_flags u64 {
   PWT :: 1 << 3;
   PCD :: 1 << 4;
   PDB :: 0x000F_FFFF_FFFF_F000;
}

CR4_Flags :: enum_flags u64 {
   VME        :: 1 <<  0;
   PVI        :: 1 <<  1;
   TSD        :: 1 <<  2;
   DE         :: 1 <<  3;
   PSE        :: 1 <<  4;
   PAE        :: 1 <<  5;
   MCE        :: 1 <<  6;
   PGE        :: 1 <<  7;
   PCE        :: 1 <<  8;
   OSFXSR     :: 1 <<  9;
   OSXMMEXCPT :: 1 << 10;
   UMIP       :: 1 << 11;
   LA57       :: 1 << 12;
   VMXE       :: 1 << 13;
   SMXE       :: 1 << 14;
   FSGSBASE   :: 1 << 16;
   PCIDE      :: 1 << 17;
   OSXXSAVE   :: 1 << 18;
   KL         :: 1 << 19;
   SMEP       :: 1 << 20;
   SMAP       :: 1 << 21;
   PKE        :: 1 << 22;
   CET        :: 1 << 23;
   PKS        :: 1 << 24;
}

CR8_Flags :: enum_flags u64 {
   TPL :: 0xF;
}

cr4_has_flags :: (flags: CR4_Flags) -> CR4_Flags {
   #asm {
      get_cr0 c:;
      and flags, c;
   }
   return flags;
}