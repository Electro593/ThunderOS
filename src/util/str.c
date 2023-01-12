/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                         **
**  Author: Aria Seiler                                                    **
**                                                                         **
**  This program is in the public domain. There is no implied warranty,    **
**  so use it at your own risk.                                            **
**                                                                         **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// #ifdef INCLUDE_HEADER

// typedef struct string {
//     c08 *Text;
//     u32 Size;
//     u32 Resizable      :  1;
//     u32 NullTerminated :  1;
//     u32 _Unused        : 31;
// } string;

// #endif



#ifdef INCLUDE_SOURCE

// #define CLStringL(Literal) CLString(Literal, sizeof(Literal))
// #define CFStringL(Literal, ...) FString(CLStringL(Literal), __VA_ARGS__)

// internal string
// CLString(c08 *Text, u32 Length)
// {
//     string Str = {0};
//     Str.Text = Text;
//     Str.Size = Length;
//     Str.Resizable = FALSE;
//     Str.NullTerminated = !Text[Length-1];
//     return Str;
// }

// internal string
// CString(c08 *Text)
// {
//     return CLString(Text, Mem_BytesUntil(Text, 0) + 1);
// }

// internal string
// VString(string Format, va_list Args)
// {
//     string Out = {0};
    
    
    
//     return Out;
// }

// internal string
// FString(string Format, ...)
// {
//     va_list Args;
//     VA_Start(Args, Format);
    
//     string Str = VString(Format, Args);
    
//     VA_End(Args);
//     return Str;
// }

// internal string
// CFString(c08 *Format, ...)
// {
//     va_list Args;
//     VA_Start(Args, Format);
    
//     string Str = VString(CString(Format), Args);
    
//     VA_End(Args);
//     return Str;
// }

internal c08 *
U64_ToStr(c08 *Buffer, u64 N, u32 Radix)
{
    persist c08 Chars[64] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-_";
    
    u64 M = N;
    u32 Len = 1;
    while(M /= Radix) Len++;
    
    if(Radix == 2) {
        Buffer[0] = '0';
        Buffer[1] = 'b';
        Len += 2;
    } else if(Radix == 8) {
        Buffer[0] = '0';
        Len += 1;
    } else if(Radix == 16) {
        Buffer[0] = '0';
        Buffer[1] = 'x';
        Len += 2;
    }
    
    Buffer[Len--] = 0;
    
    do {
        u08 Digit = N % Radix;
        Buffer[Len--] = (c08)Chars[Digit];
    } while(N /= Radix);
    
    return Buffer;
}

internal c08 *
U64_ToStrP(c08 *Buffer, u64 N, u32 Radix, u32 Width, c08 PadChar)
{
    c08 *Str = U64_ToStr(Buffer, N, Radix);
    
    c08 *C = Str;
    while(*C) C++;
    u64 Len = (u64)C - (u64)Str;
    
    if(Width > Len) {
        Mem_Cpy(Str+Width-Len, Str, Len+1);
        Mem_Set(Str, PadChar, Width-Len);
    }
    
    return Str;
}

#endif