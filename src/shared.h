/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                         **
**  Author: Aria Seiler                                                    **
**                                                                         **
**  This program is in the public domain. There is no implied warranty,    **
**  so use it at your own risk.                                            **
**                                                                         **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define _WINDOWS

#define IN
#define OUT
#define OPT
#define CONST
#define VER(...)

#ifdef _WINDOWS
#   define API __stdcall
#endif

#define ASSERT(Expression) (void)((Expression) ? 0 : *(vptr*)0)

#define global   static
#define internal static
#define external

typedef          __int8  s08;
typedef          __int16 s16;
typedef          __int32 s32;
typedef          __int64 s64;
typedef unsigned __int8  u08;
typedef unsigned __int16 u16;
typedef unsigned __int32 u32;
typedef unsigned __int64 u64;
typedef s08 b08;
typedef u16 c16;
typedef void* vptr;

#define NULL (vptr)0

typedef struct context {
    vptr (API *Allocate) (u64 Size);
    vptr (API *TempAllocate) (u64 Size);
    
    struct stack *Stack;
} context;
global context Context;