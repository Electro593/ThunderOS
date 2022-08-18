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
   Page_Full           = 0x4000000000000000,
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

// Different to avoid circular recursion. Instead of allocating extra
// page tables from AllocatePhysicalPage, it assumes there are free
// pages at Physical since invalid addresses are set as used.
internal thunderos_status
MapPAllocPage(pptr Physical, vptr *VirtualOut, u32 *UsedCountOut)
{
   b08 HasLvl5 = ((GetCR4() & CR4_57BitLinearAddress) != 0);
   pptr PhysicalStart = Physical;
   vptr EntryAddress = 0xFFFFFFFFFFFFF000;
   u32 I = !HasLvl5;
   
   for(; I < 5; I++) {
      u64 *Entry;
      u32 J = 0;
      for(; J < 512; J++) {
         Entry = (u64*)(EntryAddress | (J << 3));
         if(!(*Entry & Page_Full)) break;
      }
      
      // This will only happen if we're entirely out of virtual
      // address space, which is extremely unlikely, or the data
      // got corrupted somehow
      Assert(J != 512);
      
      if(!(*Entry & Page_Present)) {
         *Entry = Physical | Page_Present;
         Physical += 0x1000;
      }
      
      EntryAddress = (EntryAddress | (J << 3)) << 9;
   }
   
   // Ensure canonicality
   if(HasLvl5 && !(EntryAddress & 0x0100000000000000))
      EntryAddress &= 0x01FFFFFFFFFFFFFF;
   else if(!HasLvl5 && !(EntryAddress & 0x0000800000000000))
      EntryAddress &= 0x0000FFFFFFFFFFFF;
   
   if(VirtualOut)
      *VirtualOut = EntryAddress;
   
   if(UsedCountOut)
      *UsedCountOut = (Physical - PhysicalStart) >> 12;
   
   return ST_Success;
}

/*
internal thunderos_status
FindConsecutiveFreePageCount(palloc_map *PageMap, u32 PageIndex, u32 *PageCountOut)
{
   if(!PageMap || PageIndex > 16383 || !PageCountOut)
      return ST_InvalidParameter;
   
   u32 Bit = 1;
   if(PageMap->LvlE & Bit) {
      *PageCountOut = 0;
      return ST_Success;
   }
   
   u32 Lvl0Start  = (1 << 14) | PageIndex;
   u08 *Bytes = &PageMap->LvlE;
   u32 Start, End = 2;
   u08 Byte, Mask;
   
   for(s32 I = 13; I >= 0; I--) {
      End <<= 1;
      Start = Lvl0Start >> I;
      
      Byte = Bytes[Start >> 3];
      Mask = 1 << (Start & 0x7);
      if(Byte & Mask) {
         *PageCountOut = 0;
         return ST_Success;
      }
      
      for(u32 J = Start + 1; J < End; J++) {
         Byte = Bytes[J >> 3];
         Mask = 1 << (J & 0x7);
         if(Byte & Mask) End = J;
      }
   }
   
   *PageCountOut = End - Lvl0Start;
   return ST_Success;
}

internal thunderos_status
FindConsecutiveFreeEntryCount(palloc_dir_map *DirMap,
                              u32 DirIndexS, u32 EntryIndexS,
                              u32 TableIndexS, u32 MapIndexS,
                              u32 PageIndexS, u64 *PageCountOut)
{
   if(!DirMap || DirIndex > 255 || EntryIndex > 15 || !EntryCountOut)
      return ST_InvalidParameter;
   
   u32 Bit = 1;
   if(DirMap->LvlE & Bit) {
      *EntryCountOut = 0;
      return ST_Success;
   }
   
   u64 PageCount = 0;
   
   palloc_dir *DirS, *DirE;
   palloc_dir_entry *EntryS, *EntryE;
   palloc_table *TableS, *TableE;
   palloc_map *PageMapS, *PageMapE;
   u32 DirIndexE, EntryIndexE, TableIndexE, MapIndexE;
   
   if(DirMap->Dirs[DirIndexS] & PAlloc_Present) {
      DirS = (vptr)(DirMap->Dirs[DirIndexS] & 0xFFFFFFFFFFFFF000);
      
      if(DirS->Entries[EntryIndexS] & PAlloc_Present) {
         EntryS = DirS->Entries + EntryIndexS;
         
         if(EntryS->Tables[TableIndexS] & PAlloc_Present) {
            TableS = (vptr)(EntryS->Tables[TableIndexS] & 0xFFFFFFFFFFFFF000);
            
            if(TableS->PageMaps[MapIndexS] & PAlloc_Present) {
               PageMapS = (vptr)(TableS->PageMaps[MapIndexS] & 0xFFFFFFFFFFFFF000);
               
               
            }
         }
      }
   }
   
   if(DirMap->Dirs[StartDirIndex] & PAlloc_Present) {
      palloc_dir *StartDir = (vptr)(DirMap->Dirs[StartDirIndex] & 0xFFFFFFFFFFFFF000);
      palloc_dir *EndDir = StartDir;
      
      
      
      
      
      u32 EndEntryIndex = StartEntryIndex + 1;
      for(; EndEntryIndex < 16; EndEntryIndex++) {
         if(StartDir->Entries[EndEntryIndex] & PAlloc_Present) break;
      }
      
      // Completely free entries in the start dir
      PageCount += (EndEntryIndex - (StartEntryIndex+1)) * 0x0000010000000000;
      
      if(EndEntryIndex == 16) {
         u32 EndDirIndex = StartDirIndex + 1;
         for(; EndDirIndex < 256; EndDirIndex++) {
            if(DirMap->Dirs[EndDirIndex] & PAlloc_Present) break;
         }
         
         // Completely free dirs
         PageCount += (EndDirIndex - (StartDirIndex+1)) * 0x0000100000000000;
         
         if(EndDirIndex == 256) {
            *PageCountOut = PageCount;
            return ST_Success;
         }
         
         EndDir = (vptr)(DirMap->Dirs[EndDirIndex] & 0xFFFFFFFFFFFFF000);
         
         for(EndEntryIndex = 0; EndEntryIndex < 16; EndEntryIndex++) {
            if(EndDir->Entries[EndEntryIndex] & PAlloc_Present) break;
         }
         
         Assert(EndEntryIndex != 16);
         
         // Completely free entries in the end dir
         PageCount += EndEntryIndex * 0x0000010000000000;
      }
      
      palloc_dir_entry *EndEntry = EndDir->Entries + EndEntryIndex;
      PageCount += __PagesInEndEntry;
      
   }
}
*/

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

internal void
GetPAllocTableMapAddr(palloc_dir_entry *Entry, u64 *TableMapAddrOut)
{
   Assert(Entry && TableMapAddrOut);
   
   u32 TableMapAddr = 0;
   for(u32 I = 0; I < 32; I++)
      TableMapAddr = (TableMapAddr << 2) | ((Entry->Tables[I] >> 10) & 3);
   *TableMapAddrOut = TableMapAddr;
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

// internal void
// SetSparsePAllocPageRange(palloc_dir_map *DirMap, pptr Start, u64 Count,
//                          vptr *ExtraPages, u32 *ExtraPageCount)
// {
//    pptr End = Start + ((Count - 1) << 12);
   
//    Assert(DirMap && End < (1 << 52) && ExtraPageCount);
   
//    if(Count == 0) {
//       *ExtraPageCount = 0;
//       return;
//    }
   
//    pptr End = Start + ((PageCount - 1) << 12);
   
//    u32 MapS  = (Start >> 26) & 0x01FF;
//    u32 MapE  = (End   >> 26) & 0x01FF;
//    u32 PageS = (Start >> 14) & 0x3FFF;
//    u32 PageE = (End   >> 14) & 0x3FFF;
   
//    u32 TotalPageMaps = MapE - MapS + 1;
   
//    b08 NeedTable = (Entry->Tables[TableIndex] & PAlloc_Present) == 0;
   
//    u64 TableMapAddr;
//    GetPAllocMapAddr(Entry, &TableMapAddr);
//    b08 NeedTableMap = (TableMapAddr & PAlloc_Present) == 0;
   
//    // If the entire map is nonexistant, 
//    Assert(!(NeedTableMap && !NeedTable));
   
//    u32 NeededPages;
//    if(NeedTable) {
//       NeededPages = Need + 1 + TotalPageMaps;
      
      
//    }
//    if(*ExtraPageCount < NeededPages) {
//       *ExtraPageCount = NeededPages;
//       return;
//    }
   
   
   
// }

internal thunderos_status
FindFreePAllocDirEntry(palloc_dir_map *DirMap, u32 *DirIndexOut, u32 *EntryIndexOut)
{
   if(!DirMap || !DirIndexOut || !EntryIndexOut) return ST_InvalidParameter;
   
   u32 Bit = 1;
   if(DirMap->LvlE & Bit) return ST_NotFound;
   
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
   
   return ST_Success;
}

internal thunderos_status
FindFreePAllocPageMap(palloc_dir_entry *Entry, palloc_map *TableMap, u32 *TableIndexOut, u32 *MapIndexOut)
{
   if(!Entry || !TableMap || !TableIndexOut || !MapIndexOut) return ST_InvalidParameter;
   
   u32 Bit = 1;
   u08 *Bytes = &TableMap->LvlE;
   u08 Byte, Mask;
   
   if(TableMap->LvlE & Bit) return ST_NotFound;
   
   for(u32 I = 0; I < 14; I++) {
      Bit <<= 1;
      Byte = Bytes[Bit >> 3];
      Mask = 1 << (Bit & 0x7);
      if(Byte & Mask) Bit++;
   }
   
   Assert(!((Bit & 1) && (Byte & (Mask<<1))));
   
   *TableIndexOut = (Bit >> 9) & 0x1F; // 16384..32767 -> 32..63 -> 0..31
   *MapIndexOut = (Bit & 0x1FF);
   
   return ST_Success;
}

internal thunderos_status
FindFreePAllocPage(palloc_map *PageMap, u32 *PageIndexOut)
{
   if(!PageMap || !PageIndexOut) return ST_InvalidParameter;
   
   u32 Bit = 1;
   u08 *Bytes = &PageMap->LvlE;
   u08 Byte, Mask;
   
   if(!(PageMap->LvlE & Bit)) return ST_NotFound;
   
   for(u32 I = 0; I < 14; I++) {
      Bit <<= 1;
      Byte = Bytes[Bit >> 3];
      Mask = 1 << (Bit & 0x7);
      if(Byte & Mask) Bit++;
   }
   
   Assert(!((Bit & 1) && (Byte & (Mask<<1))));
   
   *PageIndexOut = Bit & 0x3FFF; // 16384..32767 -> 0..16383
   
   return ST_Success;
}

internal thunderos_status
FindFreePhysicalPage(palloc_dir_map *DirMap, pptr *PageOut)
{
   if(!DirMap || !PageOut) return ST_InvalidParameter;
   
   thunderos_status Status;
   u32 DirCount, EntryCount, TableCount, MapCount, PageCount;
   
   palloc_dir *Dir;
   palloc_dir_entry *Entry;
   palloc_map *TableMap, *PageMap;
   palloc_table *Table;
   u32 DirIndex, EntryIndex, TableIndex, MapIndex, PageIndex;
   
   // Find the palloc_dir and palloc_dir_entry
   {
      Status = FindFreePAllocDirEntry(DirMap, &DirIndex, &EntryIndex);
      Assert(Status == ST_Success);
      
      if(!(DirMap->Dirs[DirIndex] & PAlloc_Present)) {
         *PageOut = (DirIndex << 44);
         return ST_Success;
      }
      
      Dir = (vptr)((u64)DirMap->Dirs[DirIndex] & 0xFFFFFFFFFFFFF000);
      Entry = Dir->Entries + EntryIndex;
   }
   
   // Find the 'table' palloc_map
   {
      u64 TableMapAddr;
      GetPAllocTableMapAddr(Entry, &TableMapAddr);
      
      if(!(TableMapAddr & PAlloc_Present)) {
         *PageOut = (DirIndex << 44) | (EntryIndex << 40);
         return ST_Success;
      }
      
      TableMap = (vptr)(TableMapAddr & 0xFFFFFFFFFFFFF000);
   }
   
   // Find the palloc_table and 'page' palloc_map
   {
      Status = FindFreePAllocPageMap(Entry, TableMap, &TableIndex, &MapIndex);
      Assert(Status == ST_Success);
      
      if(!(Entry->Tables[TableIndex] & PAlloc_Present)) {
         *PageOut = (DirIndex << 44) | (EntryIndex << 40) | (TableIndex << 35);
         return ST_Success;
      }
      
      Table = (vptr)(Entry->Tables[TableIndex] & 0xFFFFFFFFFFFFF000);
      
      if(!(Table->PageMaps[MapIndex] & PAlloc_Present)) {
         *PageOut = (DirIndex << 44) | (EntryIndex << 40) | (TableIndex << 35) | (MapIndex << 26);
         return ST_Success;
      }
      
      PageMap = (vptr)(Table->PageMaps[MapIndex] & 0xFFFFFFFFFFFFF000);
   }
   
   // Find the page
   {
      Status = FindFreePAllocPage(PageMap, &PageIndex);
      Assert(Status == ST_Success);
      
      *PageOut = (DirIndex << 44) | (EntryIndex << 40) | (TableIndex << 35) | (MapIndex << 26) | (PageIndex << 12);
      return ST_Success;
   }
}

internal thunderos_status
AllocatePhysicalPage(palloc_dir_map *DirMap, pptr *PageOut)
{
   if(!DirMap || !PageOut) return ST_InvalidParameter;
   
   thunderos_status Status;
   u32 PageCount = 0;
   
   palloc_dir *Dir;
   palloc_dir_entry *Entry;
   palloc_map *TableMap, *PageMap;
   palloc_table *Table;
   u32 DirIndex, EntryIndex, TableIndex, MapIndex, PageIndex;
   
   // Find Dir and Entry
   {
      Status = FindFreePAllocDirEntry(DirMap, &DirIndex, &EntryIndex);
      Assert(Status == ST_Success);
      
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
      u64 TableMapAddr;
      GetPAllocTableMapAddr(Entry, &TableMapAddr);
      
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
      Status = FindFreePAllocPageMap(Entry, TableMap, &TableIndex, &MapIndex);
      Assert(Status == ST_Success);
      
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
      Status = FindFreePAllocPage(PageMap, &PageIndex);
      Assert(Status == ST_Success);
      
      *PageOut = (DirIndex << 44) | (EntryIndex << 40) | (TableIndex << 35) | (MapIndex << 26) | (PageIndex << 12);
      
      u32 Bit = (1 << 14) | PageIndex;
      for(u32 I = 0; I <= 14; I++) {
         u08 *Byte = &PageMap->LvlE + (Bit >> 3);
         u08 Mask1 = 1 << (Bit & 0x7);
         u08 Mask2 = 3 << (Bit & 0x6);
         *Byte |= Mask1;
         if((*Byte & Mask2) != Mask2) break;
         Bit >>= 1;
      }
      if(!(PageMap->LvlE & 2)) return ST_Success;
      
      Bit = (1 << 14) | (TableIndex << 9) | MapIndex;
      for(u32 I = 0; I <= 14; I++) {
         u08 *Byte = &TableMap->LvlE + (Bit >> 3);
         u08 Mask1 = 1 << (Bit & 0x7);
         u08 Mask2 = 3 << (Bit & 0x6);
         *Byte |= Mask1;
         if((*Byte & Mask2) != Mask2) break;
         Bit >>= 1;
      }
      if(!(TableMap->LvlE & 2)) return ST_Success;
      
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
      Assert(PageCount < 16383);
      
      // If any other pages were allocated, PageMap should be new, so it
      // should be empty. Therefore, just set the first bits.
      
      // Allocate the original page while we're at it.
      *PageOut = (DirIndex << 44) | (EntryIndex << 40) | (TableIndex << 35) | (MapIndex << 26) | (PageCount << 12);
      PageCount++;
      
      SetPAllocPageMapRange(PageMap, 0, PageCount);
   }
   
   return ST_Success;
}

internal void
FreePhysicalPage(palloc_dir_map *DirMap, pptr Page)
{
   u32 DirIndex   = (Page >> 44) & 0x00FF;
   u32 EntryIndex = (Page >> 40) & 0x000F;
   u32 TableIndex = (Page >> 35) & 0x001F;
   u32 MapIndex   = (Page >> 26) & 0x01FF;
   u32 PageIndex  = (Page >> 12) & 0x3FFF;
   Page &= 0xFFFFFFFFFFFFF000;
   
   u32 DirMapIndex   = (Page >> 40) & 0x0FFF;
   u32 TableMapIndex = (Page >> 26) & 0x3FFF;
   u32 PageMapIndex  = (Page >> 12) & 0x3FFF;
   
   thunderos_status Status;
   
   palloc_dir *Dir;
   if(!(DirMap->Dirs[DirIndex] & PAlloc_Present)) {
      // It must have been set as full without a page. Otherwise,
      // we're freeing a non-allocated page, which shouldn't happen
      Assert(DirMap->Lvl4[DirIndex >> 3] & (DirIndex & 7));
      
      u64 DirAddr;
      b08 Reallocated = FALSE;
      Status = AllocatePhysicalPage(DirMap, &DirAddr);
      if(Status == ST_NotFound) {
         DirAddr = Page;
         Reallocated = TRUE;
      }
      DirMap->Dirs[DirIndex] = DirAddr | PAlloc_Present;
      Dir = (vptr)DirAddr;
      Mem_Set(Dir, 0, sizeof(palloc_dir));
      
      if(Reallocated) return;
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
      
      b08 Reallocated = FALSE;
      Status = AllocatePhysicalPage(DirMap, &TableMapAddr);
      if(Status == ST_NotFound) {
         TableMapAddr = Page;
         Reallocated = TRUE;
      }
      TableMap = (vptr)TableMapAddr;
      SetPAllocTableMapAddr(Entry, TableMapAddr | PAlloc_Present);
      Mem_Set(TableMap, 255, sizeof(palloc_map));
      TableMap->LvlE ^= 1;
      
      if(Reallocated) return;
   } else {
      TableMap = GetPAllocTableMap(TableMapAddr);
   }
   
   palloc_table *Table;
   if(!(Entry->Tables[TableIndex] & PAlloc_Present)) {
      // Same with Dir and TableMap
      Assert(TableMap->Lvl9[TableIndex >> 3] & (TableIndex & 7));
      
      u64 TableAddr;
      b08 Reallocated = FALSE;
      Status = AllocatePhysicalPage(DirMap, &TableAddr);
      if(Status == ST_NotFound) {
         TableAddr = Page;
         Reallocated = TRUE;
      }
      Entry->Tables[TableIndex] = TableAddr | PAlloc_Present;
      Table = (vptr)TableAddr;
      Mem_Set(Table, 0, sizeof(palloc_table));
      
      if(Reallocated) return;
   } else {
      Table = GetPAllocTable(Entry, TableIndex);
   }
   
   palloc_map *PageMap;
   if(!(Table->PageMaps[MapIndex] & PAlloc_Present)) {
      // Same with Dir, TableMap, and Table
      Assert(TableMap->Lvl9[TableMapIndex >> 3] & (TableMapIndex & 7));
      
      u64 PageMapAddr;
      b08 Reallocated = FALSE;
      Status = AllocatePhysicalPage(DirMap, &PageMapAddr);
      if(Status == ST_NotFound) {
         PageMapAddr = Page;
         Reallocated = TRUE;
      }
      Table->PageMaps[MapIndex] = PageMapAddr | PAlloc_Present;
      PageMap = (vptr)PageMapAddr;
      Mem_Set(PageMap, 255, sizeof(palloc_map));
      PageMap->LvlE ^= 1;
      
      if(Reallocated) return;
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
   
   //TODO: Delete empty pages
}

#endif