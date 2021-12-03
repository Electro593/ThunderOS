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
    u32 Size;
} str;

internal u32
Str_Len(c16 *String)
{
    u32 Length;
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
    Result.Size = Str_Len(String);
    return Result;
}

internal str
Print(str String,
      ...)
{
    str Result = {0};
    
    c16 *C = String.Data;
    for(u32 Index = 0;
        Index < String.Size;
        Index++)
    {
        
    }
}