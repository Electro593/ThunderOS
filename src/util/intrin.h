/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                         **
**  Author: Aria Seiler                                                    **
**                                                                         **
**  This program is in the public domain. There is no implied warranty,    **
**  so use it at your own risk.                                            **
**                                                                         **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifdef INCLUDE_HEADER

#ifdef _X64

#if defined(_MSVC)
    typedef u08* va_list;
    void __va_start(va_list *Args, ...);
    #define VA_Start(Args, Last) ((void)(__va_start(&Args, 0)))
    #define VA_End(Args) ((void)(Args = NULL))
    #define VA_Next(Args, Type) \
        ((sizeof(Type) > 8 || (sizeof(Type) & (sizeof(Type) - 1)) != 0) /*Pointer or not a power of 2*/ \
            ? **(Type**)((Args += 8) - 8) \
            :  *(Type* )((Args += 8) - 8))
#elif defined(_GCC)
    typedef __builtin_va_list va_list;
    #define VA_Start(Args, Last) __builtin_va_start(Args, Last)
    #define VA_End(Args) __builtin_va_end(Args)
    #define VA_Next(Args, Type) __builtin_va_arg(Args, Type)
    
    typedef r64 r128  __attribute__((vector_size(16), __may_alias__));
    typedef s64 s128  __attribute__((vector_size(16), __may_alias__));
    typedef u64 u128  __attribute__((vector_size(16), __may_alias__));
    typedef r64 r128u __attribute__((vector_size(16), __may_alias__, __aligned__(1)));
    typedef s64 s128u __attribute__((vector_size(16), __may_alias__, __aligned__(1)));
    typedef u64 u128u __attribute__((vector_size(16), __may_alias__, __aligned__(1)));
    typedef r32 r32x4  __attribute__((vector_size(16)));
    typedef r64 r64x2  __attribute__((vector_size(16)));
    typedef s08 s08x16 __attribute__((vector_size(16)));
    typedef s16 s16x8  __attribute__((vector_size(16)));
    typedef s32 s32x4  __attribute__((vector_size(16)));
    typedef s64 s64x2  __attribute__((vector_size(16)));
    typedef u08 u08x16 __attribute__((vector_size(16)));
    typedef u16 u16x8  __attribute__((vector_size(16)));
    typedef u32 u32x4  __attribute__((vector_size(16)));
    typedef u64 u64x2  __attribute__((vector_size(16)));
    
    #define R128_Set_4x32(_0,_1,_2,_3) ((r32x4){_0,_1,_2,_3})
    #define R128_Set1(_0)            ((r128){_0, _0, _0, _0})
    #define R128_Sqrt_4(V)           (__builtin_ia32_sqrtps(V))
    #define R128_ToU128(V)           (__builtin_convertvector(V, u128))
    
    #define S128_Set1(_0)          ((s128){_0, _0, _0, _0})
    #define S128_Stream(P, V)      (__builtin_ia32_movntdq((_s128_2x64*)(P), (_s128_2x64)(V)))
    #define S128_MaskMove(V, M, P) (__builtin_ia32_maskmovdqu((_s128_16x8)V, (_s128_16x8)M, (s08*)P))
    #define S128_ToR128(V)         (__builtin_convertvector(V, r128))
    
    #define U128_Set(_0, _1, _2, _3) ((u128){_0, _1, _2, _3})
    #define U128_Set1(_0)            ((u128){_0, _0, _0, _0})
    #define U128_Set_1x8(_0)         ((u128)(u08x16){_0, _0, _0, _0, _0, _0, _0, _0})
    #define U128_Stream(P, V)        S128_Stream(P, V)
    #define U128_MaskMove(V, M, P)   S128_MaskMove(V, M, P)
#endif

#endif

#endif