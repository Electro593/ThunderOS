/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                           **
**  Author: Aria Seiler                                                      **
**                                                                           **
**  This program is in the public domain. There is no implied warranty,      **
**  so use it at your own risk.                                              **
**                                                                           **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef _UTIL_MEM_H
#define _UTIL_MEM_H

#include <shared.h>

#define HEAP(type) heap_handle *

typedef vptr heap;
typedef struct heap_handle {
	u08 *Data;
	u64	 Index	  : 16;
	u64	 Offset	  : 46;
	u64	 Free	  : 1;
	u64	 Anchored : 1;
	u32	 Size;
	u16	 PrevFree;
	u16	 NextFree;
	u16	 PrevUsed;
	u16	 NextUsed;
	u16	 PrevBlock;
	u16	 NextBlock;
} heap_handle;

vptr Mem_Set(vptr Dest, u08 Data, s64 Size);
vptr Mem_Cpy(vptr Dest, vptr Src, u64 Size);
s32	 Mem_Cmp(vptr A, vptr B, u64 Size);

#endif
