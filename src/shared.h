/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                         **
**  Author: Aria Seiler                                                    **
**                                                                         **
**  This program is in the public domain. There is no implied warranty,    **
**  so use it at your own risk.                                            **
**                                                                         **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define _X64
#define _GCC

#define IN
#define OUT
#define OPT
#define CONST
#define VER(...)

#define UNUSED(...) ((void)(__VA_ARGS__))

#if defined(_MSVC)
#   define API __stdcall
#elif defined(_GCC)
#   define API
#endif

#define STATIC_ASSERT(Expression, Message) _Static_assert(Expression, Message);
#define ASSERT(Expression) (void)((Expression) ? 0 : *(vptr*)0)
#define NO_DEFAULT default: { ASSERT(FALSE); }

#define global   static
#define internal static
#define external

#if defined(_MSVC)
    typedef          __int8  s08;
    typedef          __int16 s16;
    typedef          __int32 s32;
    typedef          __int64 s64;
    typedef unsigned __int8  u08;
    typedef unsigned __int16 u16;
    typedef unsigned __int32 u32;
    typedef unsigned __int64 u64;
#else
    typedef signed   char      s08;
    typedef signed   short     s16;
    typedef signed   int       s32;
    typedef signed   long long s64;
    typedef unsigned char      u08;
    typedef unsigned short     u16;
    typedef unsigned int       u32;
    typedef unsigned long long u64;
    
    STATIC_ASSERT((sizeof(u08) == 1) && (sizeof(u16) == 2) &&
                  (sizeof(u32) == 4) && (sizeof(u64) == 8),
                  "Int sizes are incorrect");
#endif

typedef s08 b08;
typedef u08 c08;
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

/*
cd EFI/BOOT

add-symbol-file ./build/ThunderOS_Debug.efi 0x63441000 -s .rdata 0x63442000 -s .pdata 0x63443000 -s .xdata 0x63444000 -s .bss 0x63445000 -s .edata 0x63446000 -s .idata 0x63447000
add-symbol-file ./build/ThunderOS_Debug.efi 0x63441000 -o 0x3E607000 -s .rdata 0x63442000 -s .pdata 0x63443000 -s .xdata 0x63444000 -s .bss 0x63445000 -s .edata 0x63446000 -s .idata 0x63447000
add-symbol-file ./build/ThunderOS_Debug.efi 0xA1A48000 -o 0x3E607000 -s .rdata 0x63442000 -s .pdata 0x63443000 -s .xdata 0x63444000 -s .bss 0x63445000 -s .edata 0x63446000 -s .idata 0x63447000
set architecture i386:x86-64:intel
target remote :1234
*/

// internal void Convert(vptr Out, type TypeOut, vptr In, type TypeIn);

// #include <util/intrin.h>
// #include <util/mem.c>
// #include <util/str.c>

// internal void
// Convert(vptr Out,
//         type TypeOut,
//         vptr In,
//         type TypeIn)
// {
//     u64 UIntValue = 0;
    
//     switch(TypeIn)
//     {
//         case Type_U08:
//             UIntValue = (u64)*(u08*)In;
//             goto is_uint;
//         case Type_U16:
//             UIntValue = (u64)*(u16*)In;
//             goto is_uint;
//         case Type_U32:
//             UIntValue = (u64)*(u32*)In;
//             goto is_uint;
//         case Type_U64:
//             UIntValue = (u64)*(u64*)In;
            
//             is_uint:
//             switch(TypeOut)
//             {
//                 case Type_Str:
//                 {
//                     str Result;
//                     Result.Capacity = 20; // Max length of unsigned 64-bit base 10
//                     Result.Data = Context.Allocate(Result.Capacity);
                    
//                     u32 Index = Result.Capacity;
//                     do {
//                         Result.Data[--Index] = (UIntValue % 10) + L'0';
//                         UIntValue /= 10;
//                     } while(UIntValue > 0);
                    
//                     Result.Length = Result.Capacity - Index;
//                     Result.Data += Index;
                    
//                     *(str*)Out = Result;
//                 } break;
                
//                 NO_DEFAULT;
//             }
            
//             break;
        
//         NO_DEFAULT;
//     }
// }