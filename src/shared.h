/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                         **
**  Author: Aria Seiler                                                    **
**                                                                         **
**  This program is in the public domain. There is no implied warranty,    **
**  so use it at your own risk.                                            **
**                                                                         **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define _WINDOWS
#define _X64

#define IN
#define OUT
#define OPT
#define CONST
#define VER(...)

#define UNUSED(...) ((void)(__VA_ARGS__))

#ifdef _WINDOWS
#   define API __stdcall
#endif

#define ASSERT(Expression) (void)((Expression) ? 0 : *(vptr*)0)
#define NO_DEFAULT default: { ASSERT(FALSE); }

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

#define FALSE 0
#define TRUE  1
#define NULL (vptr)0
#define EQUAL 0
#define GREATER 1
#define LESS -1

typedef struct context {
    vptr (API *Allocate) (u64 Size);
    
    struct stack *Stack;
    
    struct context *PrevContext;
} context;
global context Context;

typedef enum type {
    Type_U08,
    Type_U16,
    Type_U32,
    Type_U64,
    Type_Str,
} type;

internal void Convert(vptr Out, type TypeOut, vptr In, type TypeIn);

#include <util/intrin.h>
#include <util/mem.c>
#include <util/str.c>

internal void
Convert(vptr Out,
        type TypeOut,
        vptr In,
        type TypeIn)
{
    u64 UIntValue = 0;
    
    switch(TypeIn)
    {
        case Type_U08:
            UIntValue = (u64)*(u08*)In;
            goto is_uint;
        case Type_U16:
            UIntValue = (u64)*(u16*)In;
            goto is_uint;
        case Type_U32:
            UIntValue = (u64)*(u32*)In;
            goto is_uint;
        case Type_U64:
            UIntValue = (u64)*(u64*)In;
            
            is_uint:
            switch(TypeOut)
            {
                case Type_Str:
                {
                    str Result;
                    Result.Capacity = 20; // Max length of unsigned 64-bit base 10
                    Result.Data = Context.Allocate(Result.Capacity);
                    
                    u32 Index = Result.Capacity;
                    do {
                        Result.Data[--Index] = (UIntValue % 10) + L'0';
                        UIntValue /= 10;
                    } while(UIntValue > 0);
                    
                    Result.Length = Result.Capacity - Index;
                    Result.Data += Index;
                    
                    *(str*)Out = Result;
                } break;
                
                NO_DEFAULT;
            }
            
            break;
        
        NO_DEFAULT;
    }
}