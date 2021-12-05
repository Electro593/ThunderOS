/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                         **
**  Author: Aria Seiler                                                    **
**                                                                         **
**  This program is in the public domain. There is no implied warranty,    **
**  so use it at your own risk.                                            **
**                                                                         **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

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
#endif

#endif