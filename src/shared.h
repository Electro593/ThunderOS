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
#define ASSERT(Expression) { if(!(Expression)) asm volatile ("int $3"); }
#define Assert(Expression) { if(!(Expression)) KernelError(__FILE__, __LINE__, #Expression); }
#define NO_DEFAULT default: { ASSERT(FALSE); }
#define OFFSETOF(Type, Field) ((u64)&((Type*)0)->Field)
#define SIZEOF(Type, Field)  (sizeof(((Type*)0)->Field))
#define SWAP(A, B) { typeof(A) Temp = A; A = B; B = Temp; }
#define LITERAL_CAST(EndType, StartType, ...) (*(EndType*)&(StartType){__VA_ARGS__})
#define FORCE_CAST(Type, ...) (*(Type*)&(__VA_ARGS__))
#define BIT_CLEAR(Bitstring, Index) ((Bitstring) & ~(1ULL << (Index)))
#define INDEX_2D(X, Y, Width) ((X) + (Y)*(Width))

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

typedef float r32;
typedef double r64;
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
    struct terminal *Terminal;
    
    u32 *Framebuffer;
    u64 FramebufferSize;
    
    struct context *PrevContext;
} context;
global context Context;

typedef enum type {
    Type_C08p,
    Type_Str,
    Type_U08,
    Type_U16,
    Type_U32,
    Type_U64,
} type;