/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                         **
**  Author: Aria Seiler                                                    **
**                                                                         **
**  This program is in the public domain. There is no implied warranty,    **
**  so use it at your own risk.                                            **
**                                                                         **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifdef INCLUDE_SOURCE

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

// typedef struct str {
//     c16 *Data;
//     u32 Length;
//     u32 Capacity;
// } str;

// internal u32
// Str_Len(c16 *String)
// {
//     u32 Length = 0;
//     while(*String++ != 0) {
//         Length++;
//     }
    
//     return Length;
// }

// internal str
// Str(c16 *String)
// {
//     str Result;
//     Result.Data = String;
//     Result.Length = Str_Len(String) + 1;
//     Result.Capacity = Result.Length;
//     return Result;
// }

// #define Printc(c16_p__String, ...) Print(Str(c16_p__String), __VA_ARGS__)
// internal str
// Print(str String,
//       ...)
// {
//     va_list Args;
//     VA_Start(Args, String);
    
//     str Result;
//     Result.Length = 0;
//     Result.Capacity = 64;
//     Result.Data = Context.Allocate(Result.Capacity * sizeof(c16));
    
//     c16 *C = String.Data;
//     for(u32 CI = 0;
//         CI < String.Length;
//         CI++)
//     {
//         if(*C == L'$')
//         {
//             if(CI > 0 && *(C-1) == L'\\') // Escaped, just overwrite the backslash
//                 Result.Data[Result.Length - 1] = *C;
//             else
//             {
//                 str Addend = {0};
                
//                 u32 Index = 0;
//                 c16 Buffer[32];
//                 C++;
//                 while(*C != L'$')
//                     Buffer[Index++] = *C++;
//                 Assert(Index <= sizeof(Buffer) / sizeof(Buffer[0]));
                
//                 if(Mem_Cmp(Buffer, L"c08*", Index) == EQUAL)
//                 {
//                     c08 *Value = VA_Next(Args, c08*);
//                     Convert(&Addend, Type_Str, &Value, Type_C08p);
//                 }
//                 else if(Mem_Cmp(Buffer, L"u32", Index) == EQUAL)
//                 {
//                     u32 Value = VA_Next(Args, u32);
//                     Convert(&Addend, Type_Str, &Value, Type_U32);
//                 }
                
//                 if(Result.Length + Addend.Length >= Result.Capacity)
//                 {
//                     c16 *OldData = Result.Data;
//                     while(Result.Length + Addend.Length >= Result.Capacity)
//                         Result.Capacity *= 2;
                    
//                     Result.Data = Context.Allocate(Result.Capacity * sizeof(c16));
//                     Mem_Cpy(Result.Data, OldData, Result.Length * sizeof(c16));
//                 }
                
//                 Mem_Cpy(Result.Data + Result.Length, Addend.Data, Addend.Length * sizeof(c16));
//                 Result.Length += Addend.Length;
//             }
//         }
//         else
//         {
//             if(Result.Length == Result.Capacity)
//             {
//                 c16 *OldData = Result.Data;
//                 Result.Capacity *= 2;
//                 Result.Data = Context.Allocate(Result.Capacity * sizeof(c16));
//                 Mem_Cpy(Result.Data, OldData, Result.Length * sizeof(c16));
//             }
            
//             Result.Data[Result.Length++] = *C;
//         }
        
//         C++;
//     }
    
//     VA_End(Args);
//     return Result;
// }