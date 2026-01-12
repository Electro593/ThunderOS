/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                           **
**  Author: Aria Seiler                                                      **
**                                                                           **
**  This program is in the public domain. There is no implied warranty,      **
**  so use it at your own risk.                                              **
**                                                                           **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef _UTIL_STR_H
#define _UTIL_STR_H

#include <shared.h>

c08 *U64_ToStr(c08 *Buffer, u64 N, u32 Radix);
c08 *U64_ToStrP(c08 *Buffer, u64 N, u32 Radix, u32 Width, c08 PadChar);

#endif
