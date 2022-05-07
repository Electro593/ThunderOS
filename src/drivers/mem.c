/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                         **
**  Author: Aria Seiler                                                    **
**                                                                         **
**  This program is in the public domain. There is no implied warranty,    **
**  so use it at your own risk.                                            **
**                                                                         **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifdef INCLUDE_HEADER

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


#endif



#ifdef INCLUDE_SOURCE

#endif