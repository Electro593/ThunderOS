/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                           **
**  Author: Aria Seiler                                                      **
**                                                                           **
**  This program is in the public domain. There is no implied warranty,      **
**  so use it at your own risk.                                              **
**                                                                           **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef _DRIVERS_MEM_H
#define _DRIVERS_MEM_H

#include <shared.h>

typedef enum vmap_flags {
	VMap_Present		= 0x0000000000000001,
	VMap_WriteAccess	= 0x0000000000000002,
	VMap_UserAccess		= 0x0000000000000004,
	VMap_WriteThrough	= 0x0000000000000008,
	VMap_CacheDisable	= 0x0000000000000010,
	VMap_Accessed		= 0x0000000000000020,
	VMap_Dirty			= 0x0000000000000040,
	VMap_PTPAT			= 0x0000000000000080,
	VMap_PageSize		= 0x0000000000000080,
	VMap_Global			= 0x0000000000000100,
	VMap_Restart		= 0x0000000000000800,
	VMap_PAT			= 0x0000000000001000,
	VMap_ProtectionKey	= 0x7800000000000000,
	VMap_ExecuteDisable = 0x8000000000000000,
} vmap_flags;

typedef struct vmap_node {
	u64 Entries[512];
} vmap_node __attribute__((aligned(4096)));

typedef struct pmap_leaf {
	// 0bxxxxyyzz, x = Lvl12, y = Lvl13
	u08 LvlE;
	u08 Lvl11[1];
	u08 Lvl10[2];
	u08 Lvl9[4];
	u08 Lvl8[8];
	u08 Lvl7[16];
	u08 Lvl6[32];
	u08 Lvl5[64];
	u08 Lvl4[128];
	u08 Lvl3[256];
	u08 Lvl2[512];
	u08 Lvl1[1024];
	u08 Lvl0[1024];
} pmap_leaf __attribute__((aligned(4096)));

typedef struct pmap_path {
	pmap_leaf *Leaf;

	u64 LeafIndex : 13;
	u64 _Unused	  : 51;

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
			u16		   LEIndex[5];
		};
	};

	u08 HasLvl5	 : 1;
	u08 Null	 : 1;
	u08 _Unused0 : 6;
	u08 _Unused1[5];

	vptr Addr;
	vptr EntryAddr;
} vmap_path;

extern pmap_leaf *PMap;
extern pmap_leaf  _PMap;
extern u64		  PMapBase;

void ClearPMapLeafRange(pmap_path Start, pmap_path End);
void UnmapPageLevel(vmap_path VPath, vmap_path VEnd, s32 Level, b08 Invalidate);
vmap_path GetVMapPathFromAddr(vptr Addr);
pmap_path GetPMapPathFromVMapPath(vmap_path VPath);

#endif
