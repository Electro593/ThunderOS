/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                         **
**  Author: Aria Seiler                                                    **
**                                                                         **
**  This program is in the public domain. There is no implied warranty,    **
**  so use it at your own risk.                                            **
**                                                                         **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifdef INCLUDE_HEADER

// Since bits 58..52 and 10..8 are always ignored, we'll use
// them for our own purposes
typedef enum page_table_flags {
   Page_Present        = 0x0000000000000001,
   Page_WriteAccess    = 0x0000000000000002,
   Page_UserAccess     = 0x0000000000000004,
   Page_WriteThrough   = 0x0000000000000008,
   Page_CacheDisable   = 0x0000000000000010,
   Page_Accessed       = 0x0000000000000020,
   Page_Dirty          = 0x0000000000000040,
   Page_PTPAT          = 0x0000000000000080,
   Page_PageSize       = 0x0000000000000080,
   Page_Global         = 0x0000000000000100,
   Page_Restart        = 0x0000000000000800,
   Page_PAT            = 0x0000000000001000,
   Page_OnDisk         = 0x0200000000000000,
   Page_Full           = 0x0400000000000000,
   Page_ExecuteDisable = 0x8000000000000000,
} page_table_flags;

// 2 MiB
typedef struct page_table {
   // |     63 | Execute Disable
   // | 62..59 | Protection Key
   // | 58..52 | _Unused
   // | 51..12 | Physical Address
   // | 11.. 9 | _Unused
   // |      8 | Global
   // |      7 | Page Attribute Table
   // |      6 | Dirty
   // |      5 | Accessed
   // |      4 | Cache Disable
   // |      3 | Write-Through
   // |      2 | User/Supervisor
   // |      1 | Read/Write
   // |      0 | Present
   u64 Entries[512];
} page_table __attribute__((aligned(4096)));

// 1 GiB
typedef struct page_directory {
   // |     63 | Execute Disable
   // | 62..52 | _Unused
   // | 51..12 | Address
   // | 11.. 8 | _Unused
   // |      7 | Page Size
   // |      6 | Dirty
   // |      5 | Accessed
   // |      4 | Cache Disable
   // |      3 | Write-Through
   // |      2 | User/Supervisor
   // |      1 | Read/Write
   // |      0 | Present
   u64 Entries[512];
} page_directory __attribute__((aligned(4096)));

// 512 GiB
typedef struct page_map_lvl3 {
   // |     63 | Execute Disable
   // | 62..52 | _Unused
   // | 51..12 | Address
   // | 11.. 8 | _Unused
   // |      7 | Page Size
   // |      6 | Dirty
   // |      5 | Accessed
   // |      4 | Cache Disable
   // |      3 | Write-Through
   // |      2 | User/Supervisor
   // |      1 | Read/Write
   // |      0 | Present
   u64 Entries[512];
} page_map_lvl3 __attribute__((aligned(4096)));

// 256 TiB
typedef struct page_map_lvl4 {
   // |     63 | Execute Disable
   // | 62..52 | _Unused
   // | 51..12 | Address
   // | 11.. 8 | _Unused
   // |      7 | _Reserved (0)
   // |      6 | Dirty
   // |      5 | Accessed
   // |      4 | Cache Disable
   // |      3 | Write-Through
   // |      2 | User/Supervisor
   // |      1 | Read/Write
   // |      0 | Present
   u64 Entries[512];
} page_map_lvl4 __attribute__((aligned(4096)));

//
// A palloc_dir_map represents the entirety of the physical address 
// space, meaning 4 PiB. It's a combinaiton of the palloc_map and
// palloc_table, allowing a binary search through 256 palloc_dir
// pointers.
//
// Each palloc_dir represents 2^32 pages, or 16 TiB. It's an array of
// 16 palloc_dir_entry structs, which each represent 1 TiB. Each entry
// stores a pointer to a palloc_map and 32 pointers to palloc_table
// structs, but since the pointers are to pages, their 12 least
// significant bits are clear. Therefore, All 33 pointers can be crammed
// into the space of 32.
//
// Each palloc_table represents 32 GiB, and they store 512 pointers to
// palloc_map structs. These maps are the bottom-most level, with each
// bit of the lowest level representing the usage status of a single
// page.
//
// A palloc_map is a tiered (buddy) bitmap, which facilitates searching
// for a free page. palloc_map structs have two separate uses:
// - The 'table' mode: Each bit represents whether a palloc_map pointed
//   to by an entry in a palloc_table is completely full. This is the
//   mode of the palloc_map structs pointed to by a palloc_dir_entry.
// - The 'page' mode: Each bit represents whether a specific page is
//   currently in use. Since palloc_map structs have 16384 bits in their
//   lowest level, each palloc_map represents 64 MiB.
//

typedef enum palloc_flags {
   PAlloc_Present = 0x01,
} palloc_flags;

typedef struct palloc_map {
   u08 LvlE; // 0bxxxxyyzw, x = Lvl12, y = Lvl13, z = Lvl14
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
   u08 Lvl0[2048];
} palloc_map __attribute__((aligned(4096)));

typedef struct palloc_table {
   palloc_map *PageMaps[512];
} palloc_table __attribute__((aligned(4096)));

typedef struct palloc_dir_entry {
   // The palloc_map data is spread between the tables, where each
   // successive 2 bits (most significant to least) are stored
   // in bit positions 10 and 11.
   palloc_table *Tables[32];
} palloc_dir_entry;

typedef struct palloc_dir {
   palloc_dir_entry Entries[16];
} palloc_dir __attribute__((aligned(4096)));

typedef struct palloc_dir_map {
   u08 LvlE; // 0bxxxxyyzw, x = Lvl10, y = Lvl11, z = Lvl12
   u08 Lvl9[  1];
   u08 Lvl8[  2];
   u08 Lvl7[  4];
   u08 Lvl6[  8];
   u08 Lvl5[ 16];
   u08 Lvl4[ 32];
   u08 Lvl3[ 64];
   u08 Lvl2[128];
   u08 Lvl1[256];
   u08 Lvl0[512];
   
   // Bit 0: Present
   palloc_dir *Dirs[256];
   
   u08 _Unused[1024];
} palloc_dir_map __attribute__((aligned(4096)));

#endif



#ifdef INCLUDE_SOURCE

#define GetPAllocDirEntry(Dir, Index)     (Dir->Entries + Index)
#define GetPAllocDir(DirMap, Index)       (vptr)(DirMap->Dirs[Index]    & 0xFFFFFFFFFFFFF000)
#define GetPAllocTableMap(Address)        (vptr)(Address                & 0xFFFFFFFFFFFFF000)
#define GetPAllocTable(Entry, Index)      (vptr)(Entry->Tables[Index]   & 0xFFFFFFFFFFFFF000)
#define GetPAllocPageMap(Table, Index)    (vptr)(Table->PageMaps[Index] & 0xFFFFFFFFFFFFF000)

// Different to avoid infinite recursion. Instead of allocating extra
// page tables from AllocatePhysicalPage, it assumes there are free
// pages at Physical since invalid addresses are set as used.
internal void
MapPAllocPage(pptr Physical, vptr *VirtualOut, u32 *UsedCountOut)
{
   Assert(!(Physical & 0xFFF) && VirtualOut && UsedCountOut);
   
   b08 HasLvl5 = ((GetCR4() & CR4_57BitLinearAddress) != 0);
   pptr PhysicalStart = Physical;
   u64 *PrevEntry;
   vptr EntryAddress = 0xFFFFFFFFFFFFF000;
   u32 I = !HasLvl5;
   
   for(; I < 5; I++) {
      u64 *Entry;
      u32 J = 0;
      for(; J < 512; J++) {
         Entry = (u64*)(EntryAddress | (J << 3));
         if(!(*Entry & Page_Full)) break;
      }
      
      // J == 512 will only happen if we're entirely out of virtual
      // address space, which is extremely unlikely, or the data
      // got corrupted somehow.
      if(I == !HasLvl5 && J == 512) {
         //TODO: Defragment
      } else if(I > !HasLvl5 && J == 511) {
         *PrevEntry |= Page_Full;
      }
      Assert(J != 512);
      
      if(!(*Entry & Page_Present)) {
         *Entry = Physical | Page_Present;
         Physical += 0x1000;
      }
      
      PrevEntry = Entry;
      EntryAddress = (EntryAddress | (J << 3)) << 9;
   }
   
   // Ensure canonicality
   if(HasLvl5 && !(EntryAddress & 0x0100000000000000))
      EntryAddress &= 0x01FFFFFFFFFFFFFF;
   else if(!HasLvl5 && !(EntryAddress & 0x0000800000000000))
      EntryAddress &= 0x0000FFFFFFFFFFFF;
   
   *VirtualOut = EntryAddress;
   *UsedCountOut = (Physical - PhysicalStart) >> 12;
}

internal void
SetPAllocPageMapRange(palloc_map *PageMap, u32 Start, u32 Count)
{
   Assert(PageMap && Start+Count <= 16384);
   if(Count == 0) return;
   
   if(Count == 16384) {
      PageMap->LvlE = 0xFE;
      Mem_Set(&PageMap->Lvl11, 255, 4095);
      return;
   }
   
   Start |= 0x4000;
   u32 ByteIS, ByteIE, *ByteS, *ByteE;
   u08 MaskS, MaskE;
   u32 End = Start + Count - 1;
   u08 *Bytes = &PageMap->LvlE;
   
   for(u32 I = 0; I < 14; I++) {
      ByteIS = Start >> 3;
      ByteIE = End   >> 3;
      MaskS =   0xFF << (Start & 7);
      MaskE = ~(0xFE << (End   & 7));
      ByteS = Bytes + ByteIS;
      if(ByteIS == ByteIE)
         *ByteS |= MaskS & MaskE;
      else {
         *ByteS |= MaskS;
         Mem_Set(ByteS+1, 255, ByteIE-ByteIS-1);
         ByteE = Bytes + ByteIE;
         *ByteE |= MaskE;
      }
      Start >>= 1;
      End >>= 1;
   }
}

internal void
SetPAllocDirMapRange(palloc_dir_map *DirMap, u32 Start, u32 Count)
{
   Assert(DirMap && Start+Count <= 4096);
   if(Count == 0) return;
   
   if(Count == 4096) {
      DirMap->LvlE = 0xFE;
      Mem_Set(&DirMap->Lvl9, 255, 1023);
      return;
   }
   
   Start |= 0x1000;
   u32 ByteIS, ByteIE, *ByteS, *ByteE;
   u08 MaskS, MaskE;
   u32 End = Start + Count - 1;
   u08 *Bytes = &PageMap->LvlE;
   
   for(u32 I = 0; I < 12; I++) {
      ByteIS = Start >> 3;
      ByteIE = End   >> 3;
      MaskS =   0xFF << (Start & 7);
      MaskE = ~(0xFE << (End   & 7));
      ByteS = Bytes + ByteIS;
      if(ByteIS == ByteIE)
         *ByteS |= MaskS & MaskE;
      else {
         *ByteS |= MaskS;
         Mem_Set(ByteS+1, 255, ByteIE-ByteIS-1);
         ByteE = Bytes + ByteIE;
         *ByteE |= MaskE;
      }
      Start >>= 1;
      End >>= 1;
   }
}

internal u64
GetPAllocTableMapAddr(palloc_dir_entry *Entry)
{
   Assert(Entry);
   
   u32 TableMapAddr = 0;
   for(u32 I = 0; I < 32; I++)
      TableMapAddr = (TableMapAddr << 2) | ((Entry->Tables[I] >> 10) & 3);
   
   return TableMapAddr;
}

internal void
SetPAllocTableMapAddr(palloc_dir_entry *Entry, u64 TableMapAddr)
{
   Assert(Entry);
   
   for(u32 I = 0; I < 32; I++) {
      Entry->Tables[I] &= 0xFFFFFFFFFFFFF3FF;
      u32 Value = (TableMapAddr >> (62 - 2*I)) & 0x3;
      Entry->Tables[I] |= Value << 10;
   }
}

internal void
SetSparsePAllocRange(palloc_dir_map *DirMap,
                     pptr AllocBase, u64 PagesToAlloc,
                     pptr ExtraBase, u64 ExtraCount,
                     u64 *AllocatedCountOut, u64 *UsedExtraCountOut)
{
   Assert(DirMap && AllocatedCountOut && UsedExtraCountOut);
   Assert(!(AllocBase & 0xFFF) && !(ExtraBase & 0xFFF));
   Assert(AllocBase + PagesToAlloc <= 0x0010000000000000);
   Assert(ExtraBase + ExtraCount <= 0x0010000000000000);
   
   *AllocatedCountOut = 0;
   *UsedExtraCountOut = 0;
   if(PagesToAlloc == 0) return;
   
   //TODO: Check for nonexistant pages
   
   pptr Start = AllocBase;
   pptr End = AllocBase + PagesToAlloc;
   
   u32 WholeDirMapStart = (Start & 0x000FFF0000000000) + 0x0000010000000000;
   u32 WholeDirMapEnd   =  End   & 0x000FFF0000000000;
   
   if(WholeDirMapStart < WholeDirMapEnd) {
      SetPAllocDirMapRange(DirMap, WholeDirMapStart, WholeDirMapEnd - WholeDirMapStart);
   }
   
   PageMapStart
   TableMapStart
   DirMapStart
   DirMapMid
   DirMapEnd
   TableMapEnd
   PageMapEnd
   
   
}

internal void
FindFreePAllocDirEntry(palloc_dir_map *DirMap, u32 *DirIndexOut, u32 *EntryIndexOut)
{
   Assert(DirMap && DirIndexOut && EntryIndexOut);
   Assert(!(DirMap->LvlE & 1));
   
   u32 Bit = 1;
   u08 *Bytes = &DirMap->LvlE;
   u08 Byte, Mask;
   
   for(u32 I = 0; I < 12; I++) {
      Bit <<= 1;
      Byte = Bytes[Bit >> 3];
      Mask = 1 << (Bit & 0x7);
      if(Byte & Mask) Bit++;
   }
   
   Assert(!((Bit & 1) && (Byte & (Mask<<1)));
   
   *DirIndexOut = (Bit >> 4) & 0xFF; // 4096..8191 -> 256..511 -> 0..255
   *EntryIndexOut = (Bit & 0xF);
}

internal void
FindFreePAllocPageMap(palloc_dir_entry *Entry, palloc_map *TableMap, u32 *TableIndexOut, u32 *MapIndexOut)
{
   Assert(Entry && TableMap && TableIndexOut && MapIndexOut);
   Assert(!(TableMap->LvlE & 1));
   
   u32 Bit = 1;
   u08 *Bytes = &TableMap->LvlE;
   u08 Byte, Mask;
   
   for(u32 I = 0; I < 14; I++) {
      Bit <<= 1;
      Byte = Bytes[Bit >> 3];
      Mask = 1 << (Bit & 0x7);
      if(Byte & Mask) Bit++;
   }
   
   Assert(!((Bit & 1) && (Byte & (Mask<<1))));
   
   *TableIndexOut = (Bit >> 9) & 0x1F; // 16384..32767 -> 32..63 -> 0..31
   *MapIndexOut = (Bit & 0x1FF);
}

internal u32
FindFreePAllocPage(palloc_map *PageMap)
{
   Assert(PageMap);
   Assert(!(PageMap->LvlE & 1));
   
   u32 Bit = 1;
   u08 *Bytes = &PageMap->LvlE;
   u08 Byte, Mask;
   
   for(u32 I = 0; I < 14; I++) {
      Bit <<= 1;
      Byte = Bytes[Bit >> 3];
      Mask = 1 << (Bit & 0x7);
      if(Byte & Mask) Bit++;
   }
   
   Assert(!((Bit & 1) && (Byte & (Mask<<1))));
   
   return Bit & 0x3FFF; // 16384..32767 -> 0..16383
}

internal pptr
AllocatePhysicalPage(palloc_dir_map *DirMap)
{
   Assert(DirMap);
   
   pptr PageOut;
   u32 PageCount = 0;
   
   palloc_dir *Dir;
   palloc_dir_entry *Entry;
   palloc_map *TableMap, *PageMap;
   palloc_table *Table;
   u32 DirIndex, EntryIndex, TableIndex, MapIndex, PageIndex;
   
   // Find Dir and Entry
   {
      if(DirMap->LvlE & 1) {
         //TODO: Allocate a swap page
         Assert(FALSE);
      }
      
      FindFreePAllocDirEntry(DirMap, &DirIndex, &EntryIndex);
      
      if(!(DirMap->Dirs[DirIndex] & PAlloc_Present)) {
         u64 PAddr = DirIndex << 44;
         u64 VAddr;
         MapPAllocPage(PAddr, &VAddr, &PageCount);
         Mem_Set(VAddr, 0, 0x1000);
         
         DirMap->Dirs[DirIndex] = VAddr | 1;
      }
      Dir = (vptr)(DirMap->Dirs[DirIndex] & 0xFFFFFFFFFFFFF000);
      
      Entry = Dir->Entries + EntryIndex;
   }
   
   // Find TableMap
   {
      u64 TableMapAddr = GetPAllocTableMapAddr(Entry);
      
      if(!(TableMapAddr & PAlloc_Present)) {
         u64 PAddr = (DirIndex << 44) | (EntryIndex << 40) | (PageCount << 12);
         u64 VAddr;
         u32 TempPageCount;
         MapPAllocPage(PAddr, &VAddr, &TempPageCount);
         Mem_Set(VAddr, 0, 0x1000);
         
         PageCount += TempPageCount;
         TableMapAddr = VAddr | 1;
         
         SetPAllocTableMapAddr(Entry, TableMapAddr);
      }
      
      TableMap = (vptr)(TableMapAddr & 0xFFFFFFFFFFFFF000);
   }
   
   // Find Table and PageMap
   {
      Assert(!(TableMap->LvlE & 1));
      FindFreePAllocPageMap(Entry, TableMap, &TableIndex, &MapIndex);
      
      Table = Entry->Tables[TableIndex];
      if(!(Entry->Tables[TableIndex] & PAlloc_Present)) {
         u64 PAddr = (DirIndex << 44) | (EntryIndex << 40) | (TableIndex << 35) | (PageCount << 12);
         u64 VAddr;
         u32 TempPageCount;
         MapPAllocPage(PAddr, &VAddr, &TempPageCount);
         Mem_Set(VAddr, 0, 0x1000);
         
         PageCount += TempPageCount;
         Entry->Tables[TableIndex] = VAddr | 1;
      }
      Table = (vptr)(Entry->Tables[TableIndex] & 0xFFFFFFFFFFFFF000);
      
      if(!(Table->PageMaps[MapIndex] & PAlloc_Present)) {
         u64 PAddr = (DirIndex << 44) | (EntryIndex << 40) | (TableIndex << 35) | (MapIndex << 26) | (PageCount << 12);
         u64 VAddr;
         u32 TempPageCount;
         MapPAllocPage(PAddr, &VAddr, &TempPageCount);
         Mem_Set(VAddr, 0, 0x1000);
         
         PageCount += TempPageCount;
         Table->PageMaps[MapIndex] = VAddr | 1;
      }
      PageMap = (vptr)(Tables->PageMaps[MapIndex] & 0xFFFFFFFFFFFFF000);
   }
   
   // Allocate the pages
   if(PageCount == 0) {
      Assert(!(PageMap->LvlE & 1));
      PageIndex = FindFreePAllocPage(PageMap);
      
      PageOut = (DirIndex << 44) | (EntryIndex << 40) | (TableIndex << 35) | (MapIndex << 26) | (PageIndex << 12);
      
      u32 Bit = (1 << 14) | PageIndex;
      for(u32 I = 0; I <= 14; I++) {
         u08 *Byte = &PageMap->LvlE + (Bit >> 3);
         u08 Mask1 = 1 << (Bit & 0x7);
         u08 Mask2 = 3 << (Bit & 0x6);
         *Byte |= Mask1;
         if((*Byte & Mask2) != Mask2) break;
         Bit >>= 1;
      }
      if(!(PageMap->LvlE & 2)) return PageOut;
      
      Bit = (1 << 14) | (TableIndex << 9) | MapIndex;
      for(u32 I = 0; I <= 14; I++) {
         u08 *Byte = &TableMap->LvlE + (Bit >> 3);
         u08 Mask1 = 1 << (Bit & 0x7);
         u08 Mask2 = 3 << (Bit & 0x6);
         *Byte |= Mask1;
         if((*Byte & Mask2) != Mask2) break;
         Bit >>= 1;
      }
      if(!(TableMap->LvlE & 2)) return PageOut;
      
      Bit = (1 << 12) | (DirIndex << 4) | EntryIndex;
      for(u32 I = 0; I <= 12; I++) {
         u08 *Byte = &DirMap->LvlE + (Bit >> 3);
         u08 Mask1 = 1 << (Bit & 0x7);
         u08 Mask2 = 3 << (Bit & 0x6);
         *Byte |= Mask1;
         if((*Byte & Mask2) != Mask2) break;
         Bit >>= 1;
      }
      
      //TODO: Some way to delete the full pages?
   } else {
      // Should really never possibly be hit, but you know. Might as well.
      Assert(PageCount < 16384);
      
      // If any other pages were allocated, PageMap should be new, so it
      // should be empty. Therefore, just set the first bits.
      
      // Allocate the original page while we're at it.
      PageOut = (DirIndex << 44) | (EntryIndex << 40) | (TableIndex << 35) | (MapIndex << 26) | (PageCount << 12);
      PageCount++;
      
      SetPAllocPageMapRange(PageMap, 0, PageCount);
   }
   
   return PageOut;
}

internal void
FreePhysicalPage(palloc_dir_map *DirMap, pptr Page)
{
   Assert(!(Page & 0xFFF));
   
   u32 DirIndex   = (Page >> 44) & 0x00FF;
   u32 EntryIndex = (Page >> 40) & 0x000F;
   u32 TableIndex = (Page >> 35) & 0x001F;
   u32 MapIndex   = (Page >> 26) & 0x01FF;
   u32 PageIndex  = (Page >> 12) & 0x3FFF;
   
   u32 DirMapIndex   = (Page >> 40) & 0x0FFF;
   u32 TableMapIndex = (Page >> 26) & 0x3FFF;
   
   palloc_dir *Dir;
   if(!(DirMap->Dirs[DirIndex] & PAlloc_Present)) {
      // It must have been set as full without a page. Otherwise,
      // we're freeing a non-allocated page, which shouldn't happen
      Assert(DirMap->Lvl4[DirIndex >> 3] & (DirIndex & 7));
      
      u64 DirAddr = AllocatePhysicalPage(DirMap);
      DirMap->Dirs[DirIndex] = DirAddr | PAlloc_Present;
      Dir = (vptr)DirAddr;
      Mem_Set(Dir, 0, sizeof(palloc_dir));
   } else {
      Dir = GetPAllocDir(DirMap, DirIndex);
   }
   
   palloc_dir_entry *Entry = GetPAllocDirEntry(Dir, EntryIndex);
   
   u64 TableMapAddr;
   palloc_map *TableMap;
   GetPAllocTableMapAddr(Entry, &TableMapAddr);
   if(!(TableMapAddr & PAlloc_Present)) {
      // Same as with Dir
      Assert(DirMap->Lvl0[DirMapIndex >> 3] & (DirMapIndex & 7));
      
      u64 TableMapAddr = AllocatePhysicalPage(DirMap, &TableMapAddr);
      TableMap = (vptr)TableMapAddr;
      SetPAllocTableMapAddr(Entry, TableMapAddr | PAlloc_Present);
      Mem_Set(TableMap, 255, sizeof(palloc_map));
      TableMap->LvlE ^= 1;
   } else {
      TableMap = GetPAllocTableMap(TableMapAddr);
   }
   
   palloc_table *Table;
   if(!(Entry->Tables[TableIndex] & PAlloc_Present)) {
      // Same with Dir and TableMap
      Assert(TableMap->Lvl9[TableIndex >> 3] & (TableIndex & 7));
      
      u64 TableAddr = AllocatePhysicalPage(DirMap);
      Entry->Tables[TableIndex] = TableAddr | PAlloc_Present;
      Table = (vptr)TableAddr;
      Mem_Set(Table, 0, sizeof(palloc_table));
   } else {
      Table = GetPAllocTable(Entry, TableIndex);
   }
   
   palloc_map *PageMap;
   if(!(Table->PageMaps[MapIndex] & PAlloc_Present)) {
      // Same with Dir, TableMap, and Table
      Assert(TableMap->Lvl9[TableMapIndex >> 3] & (TableMapIndex & 7));
      
      u64 PageMapAddr = AllocatePhysicalPage(DirMap);
      Table->PageMaps[MapIndex] = PageMapAddr | PAlloc_Present;
      PageMap = (vptr)PageMapAddr;
      Mem_Set(PageMap, 255, sizeof(palloc_map));
      PageMap->LvlE ^= 1;
   } else {
      PageMap = GetPAllocPageMap(Table, MapIndex);
   }
   
   // Now we're finally able to actually free a page
   
   // Go through PageMap
   u08 *Bytes = &PageMap->LvlE;
   u32 Bit = 0x4000 | PageIndex;
   for(u32 I = 0; I <= 14; I++) {
      u08 *Byte = Bytes + (Bit >> 3);
      u08 Mask1 = 1 << (Bit & 7);
      u08 Mask2 = 3 << (Bit & 6);
      *Byte &= ~Mask1;
      if((*Byte & Mask2) == 0) break;
      Bit >>= 1;
   }
   
   // Go through TableMap
   u08 *Bytes = &TableMap->LvlE;
   u32 Bit = 0x4000 | TableMapIndex;
   for(u32 I = 0; I <= 14; I++) {
      u08 *Byte = Bytes + (Bit >> 3);
      u08 Mask1 = 1 << (Bit & 7);
      u08 Mask2 = 3 << (Bit & 6);
      *Byte &= ~Mask1;
      if((*Byte & Mask2) == 0) break;
      Bit >>= 1;
   }
   
   // Go through DirMap
   u08 *Bytes = &DirMap->LvlE;
   u32 Bit = 0x1000 | DirMapIndex;
   for(u32 I = 0; I <= 12; I++) {
      u08 *Byte = Bytes + (Bit >> 3);
      u08 Mask1 = 1 << (Bit & 7);
      u08 Mask2 = 3 << (Bit & 6);
      *Byte &= ~Mask1;
      if((*Byte & Mask2) == 0) break;
      Bit >>= 1;
   }
   
   // We unfortunately have to iterate over the lowest level of the
   // bitmap to know if a map page is free, so instead, we'll have a
   // TODO: scheduled process that will clean up unused pages
}

internal void
FreeMapPages(u32 Lvl, u64 Start) {
   if(Lvl == 0) return;
   Assert(Lvl > 0);
   
   u64 Exp = 9*Lvl + 12;
   u64 ExpShift = 1 << Exp;
   u64 ExpMask = ~(ExpShift - 1);
   Assert(Start == (Start & ExpMask));
   
   //TODO: Handle Lvl5 optional
   
   for(u32 I = 0; I < 512; I++) {
      u64 *Entry = (u64*)(((s64)(0xFE00000000000000 | Start) >> (Exp-3)) & ~0x7);
      if(*Entry & Page_Present) {
         FreeMapPages(Lvl-1, Start | (I << (Exp-9)));
         u64 Addr = *Entry & 0x000FFFFFFFFFF000;
         FreePhysicalPage(DirMap, Addr);
      }
   }
}

internal void
FreeMapLvl(u32 Lvl, u64 Start, u64 End) {
   Assert(Lvl >= 0 && Start <= End && !(Start & 0xFFF) && !(End & 0xFFF));
   
   if(Lvl == 0) {
      u64 *Entry = (u64*)(((s64)(0xFE00000000000000 | Start) >> 9) & ~0x7);
      Mem_Set(Entry, 0, End - Start + sizeof(u64));
      return;
   }
   
   u64 Exp = 9*Lvl + 12;
   u64 ExpShift = 1 << Exp;
   u64 ExpMask = ~(ExpShift - 1);
   u64 LvlStart = (Start >> Exp) & 0x1FF;
   u64 LvlEnd = (End >> Exp) & 0x1FF;
   
   if(LvlStart == LvlEnd) {
      FreeMapLvl(Lvl-1, Start, End);
   } else {
      u64 WholeStart = (Start + ExpShift) & ExpMask;
      u64 WholeEnd = End & ExpMask;
      FreeMapLvl(Lvl-1, Start, WholeStart);
      FreeMapPages(Lvl, Start);
      Mem_Set(Entry, 0, (WholeEnd - WholeStart) >> (Exp-3));
      FreeMapLvl(Lvl-1, WholeEnd, End);
   }
}

internal void
FreeVirtualMemoryRange(vptr Base, u64 PageCount, b08 ReloadCR3)
{
   if(PageCount == 0) return;
   
   b08 HasLvl5 = ((GetCR4() & CR4_57BitLinearAddress) != 0);
   if(!HasLvl5) Start |= 0x01FF000000000000;
   
   u64 Start = Base;
   u64 End = Base + ((PageCount - 1) << 12);
   Assert(!(Start & 0xFFF) && Start <= End);
   
   FreeMapLvl(4 + HasLvl5, Start, End);
   
   if(ReloadCR3) {
      SetCR3(GetCR3());
   } else {
      Assert(FALSE && "Unimplemented!");
   }
}

#endif