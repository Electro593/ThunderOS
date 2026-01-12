/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                           **
**  Author: Aria Seiler                                                      **
**                                                                           **
**  This program is in the public domain. There is no implied warranty,      **
**  so use it at your own risk.                                              **
**                                                                           **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <util/vector.h>

DEFINE_VECTOR_INIT(3, r32)
DEFINE_VECTOR_INIT(4, u08)

DEFINE_VECTOR_ADD(3, r32)
DEFINE_VECTOR_ADD(3, u08)
DEFINE_VECTOR_ADD(4, u08)
DEFINE_VECTOR_ADD(2, u32)

DEFINE_VECTOR_SUB(3, r32)

DEFINE_VECTOR_MUL_VS(3, r32)
DEFINE_VECTOR_MUL_VS(4, r32)
DEFINE_VECTOR_MUL_VS(3, u08)
DEFINE_VECTOR_MUL_VS(4, u08)

DEFINE_VECTOR_MUL_VV(2, r32)
DEFINE_VECTOR_MUL_VV(3, r32)

DEFINE_VECTOR_DOT(3, r32)

DEFINE_VECTOR_CROSS(3, r32)

DEFINE_VECTOR_LEN(3, r32)

DEFINE_VECTOR_CAST(2, r32, u32)
DEFINE_VECTOR_CAST(2, u32, r32)
DEFINE_VECTOR_CAST(4, u08, r32)
DEFINE_VECTOR_CAST(4, r32, u08)
