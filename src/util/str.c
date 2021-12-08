/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                         **
**  Author: Aria Seiler                                                    **
**                                                                         **
**  This program is in the public domain. There is no implied warranty,    **
**  so use it at your own risk.                                            **
**                                                                         **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

typedef struct str {
    c16 *Data;
    u32 Length;
    u32 Capacity;
} str;

internal u32
Str_Len(c16 *String)
{
    u32 Length = 0;
    while(*String++ != 0) {
        Length++;
    }
    
    return Length;
}

internal str
Str(c16 *String)
{
    str Result;
    Result.Data = String;
    Result.Length = Str_Len(String) + 1;
    Result.Capacity = Result.Length;
    return Result;
}

#define Printc(c16_p__String, ...) Print(Str(c16_p__String), __VA_ARGS__)
internal str
Print(str String,
      ...)
{
    va_list Args;
    VA_Start(Args, String);
    
    str Result;
    Result.Length = 0;
    Result.Capacity = 64;
    Result.Data = Context.Allocate(Result.Capacity * sizeof(c16));
    
    c16 *C = String.Data;
    for(u32 CI = 0;
        CI < String.Length;
        CI++)
    {
        if(*C == L'$')
        {
            if(CI > 0 && *(C-1) == L'\\') // Escaped, just overwrite the backslash
                Result.Data[Result.Length - 1] = *C;
            else
            {
                str Addend = {0};
                
                u32 Index = 0;
                c16 Buffer[32];
                C++;
                while(*C != L'$')
                    Buffer[Index++] = *C++;
                ASSERT(Index <= sizeof(Buffer) / sizeof(Buffer[0]));
                
                if(Mem_Cmp(Buffer, L"c08*", Index) == EQUAL)
                {
                    c08 *Value = VA_Next(Args, c08*);
                    Convert(&Addend, Type_Str, &Value, Type_C08p);
                }
                else if(Mem_Cmp(Buffer, L"u32", Index) == EQUAL)
                {
                    u32 Value = VA_Next(Args, u32);
                    Convert(&Addend, Type_Str, &Value, Type_U32);
                }
                
                if(Result.Length + Addend.Length >= Result.Capacity)
                {
                    c16 *OldData = Result.Data;
                    while(Result.Length + Addend.Length >= Result.Capacity)
                        Result.Capacity *= 2;
                    
                    Result.Data = Context.Allocate(Result.Capacity * sizeof(c16));
                    Mem_Cpy(Result.Data, OldData, Result.Length * sizeof(c16));
                }
                
                Mem_Cpy(Result.Data + Result.Length, Addend.Data, Addend.Length * sizeof(c16));
                Result.Length += Addend.Length;
            }
        }
        else
        {
            if(Result.Length == Result.Capacity)
            {
                c16 *OldData = Result.Data;
                Result.Capacity *= 2;
                Result.Data = Context.Allocate(Result.Capacity * sizeof(c16));
                Mem_Cpy(Result.Data, OldData, Result.Length * sizeof(c16));
            }
            
            Result.Data[Result.Length++] = *C;
        }
        
        C++;
    }
    
    VA_End(Args);
    return Result;
}