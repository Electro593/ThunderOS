/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                         **
**  Author: Aria Seiler                                                    **
**                                                                         **
**  This program is in the public domain. There is no implied warranty,    **
**  so use it at your own risk.                                            **
**                                                                         **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifdef _X64

typedef u08* va_list;
void __va_start(va_list *Args, ...);
#define VA_Start(va_list__Args) ((void)(__va_start(&va_list__Args, 0)))
#define VA_End(va_list__Args) ((void)(va_list__Args = NULL))
#define VA_Arg(va_arg__Args, Type) \
    ((sizeof(Type) > 8 || (sizeof(Type) & (sizeof(Type) - 1)) != 0) /*Pointer or not a power of 2*/ \
        ? **(Type**)((va_arg__Args += 8) - 8) \
        :  *(Type* )((va_arg__Args += 8) - 8))
#define VA_Argp(va_arg__Args, Type) \
    ((sizeof(Type) > 8 || (sizeof(Type) & (sizeof(Type) - 1)) != 0) /*Pointer or not a power of 2*/ \
        ? *(Type**)((va_arg__Args += 8) - 8) \
        :  (Type* )((va_arg__Args += 8) - 8))

#endif