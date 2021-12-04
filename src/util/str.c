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
    Result.Capacity = Str_Len(String) + 1;
    return Result;
}

#define Printc(c16_p__String, ...) Print(Str(c16_p__String), __VA_ARGS__)
internal str
Print(str String,
      ...)
{
    va_list Args;
    VA_Start(Args);
    
    str Result;
    Result.Length = 0;
    Result.Capacity = 64;
    Result.Data = Context.Allocate(Result.Capacity * sizeof(c16));
    
    c16 *C = String.Data;
    for(u32 CI = 0;
        CI < String.Length;
        CI++)
    {
        if(*C == '$')
        {
            if(*(C-1) == '\\') // Escaped, just overwrite the backslash
                Result.Data[Result.Length - 1] = *C;
            else
            {
                str Addend = {0};
                
                u32 Index = 0;
                c16 Buffer[32];
                *C++;
                while(*C != '$')
                    Buffer[Index++] = *C++;
                ASSERT(Index <= sizeof(Buffer) / sizeof(Buffer[0]));
                
                if(Mem_Cmp(Buffer, "u32", Index) == EQUAL)
                    Convert(&Addend, Type_Str, VA_Argp(Args, u32), Type_U32);
                
                if(Result.Length + Addend.Length >= Result.Capacity)
                {
                    c16 *OldData = Result.Data;
                    while(Result.Length + Addend.Length >= Result.Capacity)
                        Result.Capacity *= 2;
                    
                    Result.Data = Context.Allocate(Result.Capacity * sizeof(c16));
                    Mem_Cpy(Result.Data, OldData, Result.Length);
                }
                
                Mem_Cpy(Result.Data, Addend.Data, Addend.Length);
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
                Mem_Cpy(Result.Data, OldData, Result.Length);
            }
            
            Result.Data[Result.Length++] = *C;
        }
        
        C++;
    }
    
    #undef RESIZE_IF_NECESSARY
    VA_End(Args);
    return Result;
}