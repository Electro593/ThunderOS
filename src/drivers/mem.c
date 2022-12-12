/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                         **
**  Author: Aria Seiler                                                    **
**                                                                         **
**  This program is in the public domain. There is no implied warranty,    **
**  so use it at your own risk.                                            **
**                                                                         **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifdef INCLUDE_HEADER

typedef enum vmap_flags {
   VMap_Present        = 0x0000000000000001,
   VMap_WriteAccess    = 0x0000000000000002,
   VMap_UserAccess     = 0x0000000000000004,
   VMap_WriteThrough   = 0x0000000000000008,
   VMap_CacheDisable   = 0x0000000000000010,
   VMap_Accessed       = 0x0000000000000020,
   VMap_Dirty          = 0x0000000000000040,
   VMap_PTPAT          = 0x0000000000000080,
   VMap_PageSize       = 0x0000000000000080,
   VMap_Global         = 0x0000000000000100,
   VMap_Restart        = 0x0000000000000800,
   VMap_PAT            = 0x0000000000001000,
   VMap_ProtectionKey  = 0x7800000000000000,
   VMap_ExecuteDisable = 0x8000000000000000,
} vmap_flags;

typedef struct vmap_node {
   u64 Entries[512];
} vmap_node __attribute__((aligned(4096)));

typedef struct pmap_leaf {
   // 0bxxxxyyzz, x = Lvl12, y = Lvl13
   u08 LvlE;
   u08 Lvl11[  1];
   u08 Lvl10[  2];
   u08 Lvl9[   4];
   u08 Lvl8[   8];
   u08 Lvl7[  16];
   u08 Lvl6[  32];
   u08 Lvl5[  64];
   u08 Lvl4[ 128];
   u08 Lvl3[ 256];
   u08 Lvl2[ 512];
   u08 Lvl1[1024];
   u08 Lvl0[1024];
} pmap_leaf __attribute__((aligned(4096)));

typedef struct pmap_path {
   pmap_leaf *Leaf;
   
   u64 LeafIndex : 13;
   u64 _Unused   : 51;
   
   pptr Addr;
} pmap_path;

typedef struct vmap_path {
   union {
      struct {
         vmap_node *Lvl1;
         vmap_node *Lvl2;
         vmap_node *Lvl3;
         vmap_node *Lvl4;
         vmap_node *Lvl5;
      };
      
      vmap_node *Lvl[5];
   };
   
   u64 L5EIndex :  9;
   u64 L4EIndex :  9;
   u64 L3EIndex :  9;
   u64 L2EIndex :  9;
   u64 L1EIndex :  9;
   u64 HasLvl5  :  1;
   u64 _Unused  : 18;
   
   vptr Addr;
} vmap_path;

#endif



#ifdef INCLUDE_SOURCE

// internal pmap_path
// GetFirstFreePMapPage(void)
// {
   
// }

// internal vptr
// AllocatePage(void)
// {
//    pmap_path PPath = GetFirstFreePMapPage();
//    vmap_path VPath = GetFirstFreeVMapSlot();
   
//    for(s32 I = 2 + VPath.HasLvl5; I >= 0; I--) {
//       if(!VPath.Lvl[I]) {
//          VPath = MapVMapPage(VPath, PPath, I);
//          PPath = GetNextFreePMapPage(PPath);
//       }
//    }
   
//    MapPage(VPath, PPath);
//    FillPMapLeaf(PPath);
   
//    return VPath.Addr;
// }

internal vmap_path
GetVMapPathFromAddr(vptr Addr)
{
   vmap_path VPath = {0};
   
   VPath.Addr = Addr;
   VPath.HasLvl5 = !!(GetCR4() & CR4_57BitLinearAddress);
   
   u64 Indexes[5];
   VPath.L1EIndex = Indexes[0] = ((u64)Addr >> 12) & 0x1FF;
   VPath.L2EIndex = Indexes[1] = ((u64)Addr >> 21) & 0x1FF;
   VPath.L3EIndex = Indexes[2] = ((u64)Addr >> 30) & 0x1FF;
   VPath.L4EIndex = Indexes[3] = ((u64)Addr >> 39) & 0x1FF;
   VPath.L5EIndex = Indexes[4] = ((u64)Addr >> 48) & 0x1FF;
   
   u64 *EntryAddr = (u64*)0xFFFFFFFFFFFFFFF8;
   for(u32 I = 0; I < 4 + VPath.HasLvl5; I++) {
      VPath.Lvl[I] = (vptr)EntryAddr;
      EntryAddr = (u64*)(((u64)EntryAddr << 9) | (Indexes[I] << 3));
   }
   
   return VPath;
}

internal pmap_path
GetPMapPathFromVMapPath(vmap_path VPath)
{
   pmap_path PPath = {0};
   
   u64 Entry = VPath.Lvl1->Entries[VPath.L1EIndex];
   PPath.Addr = Entry & 0x000FFFFFFFFFF000;
   PPath.Leaf = PMap;
   PPath.LeafIndex = (PPath.Addr - PMapBase) >> 12;
   
   return PPath;
}

internal void
UnmapPage(vmap_path VPath)
{
   VPath.Lvl1->Entries[VPath.L1EIndex] = 0;
   
   InvalidatePage(VPath.Addr);
}

internal void
ClearPMapLeaf(pmap_path PPath)
{
   u64 Bit = (1 << 13) | PPath.LeafIndex;
   u08 *Bytes = (u08*)PPath.Leaf;
   
   Bytes[Bit >> 3] &= ~(1 << (Bit & 7));
   u08 LRCode = (Bytes[Bit >> 3] >> (Bit & 6)) & 3;
   u08 Prev = ((LRCode >> 1) << 3) | ((LRCode >> 1) << 2) |
              ((LRCode &  1) << 1) | ((LRCode &  1) << 0);
   
   for(u32 I = 1; I < 13; I++) {
      u08 Offset = (Bit & 3) << 1;
      u08 *Byte = Bytes + (Bit >> 2);
      u08 Code = (*Byte >> Offset) & 0x3;
      
      *Byte &= ~(0b11 << Offset);
      if(Prev != 0) *Byte |= 0b01 << Offset;
      
      u08 Prev = (*Byte >> (Offset & 4)) & 0xF;
      Bit >>= 1;
   }
}

internal void
FreePage(vptr Addr)
{
   vmap_path VPath = GetVMapPathFromAddr(Addr);
   pmap_path PPath = GetPMapPathFromVMapPath(VPath);
   
   UnmapPage(VPath);
   ClearPMapLeaf(PPath);
}

#endif