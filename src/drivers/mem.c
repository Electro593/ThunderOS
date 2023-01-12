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
         
         u16 L1EIndex;
         u16 L2EIndex;
         u16 L3EIndex;
         u16 L4EIndex;
         u16 L5EIndex;
      };
      
      struct {
         vmap_node *Lvl[5];
         u16 LEIndex[5];
      };
   };
   
   u08 HasLvl5  : 1;
   u08 Null     : 1;
   u08 _Unused0 : 6;
   u08 _Unused1[5];
   
   vptr Addr;
   vptr EntryAddr;
} vmap_path;

#endif



#ifdef INCLUDE_SOURCE

internal vmap_node *
GetVMapNode(vmap_path Path, u32 Level, u32 Index)
{
   if(Path.Null || Level < 1 || Level > 5 || Index > 511)
      return NULL;
   
   u64 Entry = Path.Lvl[Level-1]->Entries[Index];
   u64 Mask = (Path.HasLvl5) ? 0x01FFFFFFFFFFF000 : 0x0000FFFFFFFFF000;
   u64 Exp = (Path.HasLvl5) ? 7 : 16;
   
   if(Entry & VMap_Present) {
      return (vptr)(((s64)Entry & Mask) << Exp >> Exp);
   }
   
   return NULL;
}

internal vmap_path
GetVMapPathFromAddr(vptr Addr)
{
   vmap_path VPath = {0};
   
   VPath.Addr = Addr;
   VPath.HasLvl5 = !!(GetCR4() & CR4_57BitLinearAddress);
   
   VPath.L1EIndex = ((u64)Addr >> 12) & 0x1FF;
   VPath.L2EIndex = ((u64)Addr >> 21) & 0x1FF;
   VPath.L3EIndex = ((u64)Addr >> 30) & 0x1FF;
   VPath.L4EIndex = ((u64)Addr >> 39) & 0x1FF;
   VPath.L5EIndex = ((u64)Addr >> 48) & 0x1FF;
   
   u64 *EntryAddr = (u64*)0xFFFFFFFFFFFFFFF8;
   for(s32 I = 4 + VPath.HasLvl5; I >= 0; I--) {
      VPath.Lvl[I] = (vptr)EntryAddr;
      EntryAddr = (u64*)(((u64)EntryAddr << 9) | (VPath.LEIndex[I] << 3));
      
      u64 Entry = VPath.Lvl[I]->Entries[VPath.LEIndex[I]];
      
      if(!(Entry & VMap_Present)) {
         VPath.Null = TRUE;
         return VPath;
      }
      
      if(Entry & VMap_PageSize) break;
   }
   
   VPath.EntryAddr = EntryAddr;
   
   return VPath;
}

internal pmap_path
GetPMapPathFromVMapPath(vmap_path VPath)
{
   pmap_path PPath = {0};
   Assert(!VPath.Null);
   
   u64 PAddrBase = *(u64*)VPath.EntryAddr & 0x000FFFFFFFFFF000;
   
   PPath.Addr = (pptr)VPath.Addr;
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
UnmapPageLevel(vmap_path VPath, vmap_path VEnd, s32 Level, b08 Invalidate);

internal void
UnmapPageLevelFromStart(vmap_path VEnd, s32 Level, b08 Invalidate)
{
   u64 Addr = (u64)VEnd.Addr & (0xFFFFFFFFFFFFF000 << (9*Level));
   vmap_path VStart = GetVMapPathFromAddr((vptr)Addr);
   UnmapPageLevel(VStart, VEnd, Level, Invalidate);
}

internal void
UnmapPageLevelToEnd(vmap_path VStart, s32 Level, b08 Invalidate)
{
   u64 Addr = (u64)VStart.Addr | (0x1FF000 << (9*Level));
   vmap_path VEnd = GetVMapPathFromAddr((vptr)Addr);
   UnmapPageLevel(VStart, VEnd, Level, Invalidate);
}

internal void
UnmapPageLevel(vmap_path VStart, vmap_path VEnd, s32 Level, b08 Invalidate)
{
   //TODO: Unmap the parent if it's empty
   
   if(VStart.Null || VEnd.Null || Level < 0 || Level > 4) return;
   
   u64 Exp = 9 * Level + 12;
   u64 Inc = 1ull << Exp;
   u64 AddrS = (u64)VStart.Addr & 0xFFFFFFFFFFFFF000;
   u64 AddrE = (u64)VEnd.Addr   & 0xFFFFFFFFFFFFF000;
   
   for(u64 I = AddrS; I <= AddrE; I += Inc) {
      vmap_path Path = GetVMapPathFromAddr((vptr)I);
      
      if(I & (Inc-1)) {
         UnmapPageLevelToEnd(Path, Level-1, Invalidate);
         I &= ~(Inc-1);
      } else if(0 < AddrE - I && AddrE - I < Inc) {
         UnmapPageLevelFromStart(Path, Level-1, Invalidate);
      } else {
         if(Level > 0 && Path.Lvl[Level])
            UnmapPageLevelToEnd(Path, Level-1, Invalidate);
         
         vmap_node *Node = GetVMapNode(Path, Level, (I >> Exp) & 0x1FF);
         Path.Lvl[Level]->Entries[I >> Exp] = 0;
         if(Invalidate) InvalidatePage((vptr)Node);
      }
   }
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
ClearPMapLeafRange(pmap_path Start, pmap_path End)
{
   Assert(Start.Leaf == End.Leaf);
   Assert(Start.LeafIndex <= End.LeafIndex);
   
   u08 *Leaf = (u08*)Start.Leaf;
   u64 S = Start.LeafIndex;
   u64 E = End.LeafIndex;
   u64 M = 7, R = 3, L = 0;
   
   //TODO: This doesn't update partial fullness properly
   
   for(u32 I = 1; E > S && I <= 13; I++) {
      u08 *Bytes = Leaf + (0x1000 >> I);
      s64 PBS = (s64) S    >> R;
      s64 FBS = (s64)(S+M) >> R;
      s64 FBE = (s64)(E-M) >> R;
      s64 PBE = (s64) E    >> R;
      
      Bytes[PBS] &= ~(0xFF << ((S & M) << L));
      Mem_Set(Bytes + FBS, 0, FBE - FBS + 1);
      Bytes[PBE] &=  (0xFF << ((E & M) << L));
      
      M = 3, R = 2, L = 1;
      S >>= 1, E >>= 1;
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