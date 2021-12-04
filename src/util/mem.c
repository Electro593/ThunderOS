/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                         **
**  Author: Aria Seiler                                                    **
**                                                                         **
**  This program is in the public domain. There is no implied warranty,    **
**  so use it at your own risk.                                            **
**                                                                         **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */



internal vptr
Mem_Cpy(vptr Dest,
        vptr Src,
        u64 Size)
{
    u08 *D08 = (u08*)Dest;
    u08 *S08 = (u08*)Src;
    while(Size--)
    {
        *D08++ = *S08++;
    }
    
    return Dest;
}

internal s32
Mem_Cmp(vptr A,
        vptr B,
        u64 Size)
{
    u08 *CA = (u08*)A;
    u08 *CB = (u08*)B;
    while(Size-- && *CA == *CB)
    {
        CA++;
        CB++;
    }
    
    if(Size == 0) return EQUAL;
    if(CA < CB)   return GREATER;
    return LESS;
}



typedef struct stack {
    u64 Size;
    vptr *FirstMarker;
    u08 *Cursor;
} stack;

internal stack *
Stack_Init(vptr Mem,
           u64 Size)
{
    stack *Result = Mem;
    Result->Size = Size - sizeof(stack);
    Result->FirstMarker = (vptr*)&Result->FirstMarker;
    Result->Cursor = (u08*)Mem + sizeof(stack) + sizeof(vptr);
    return Result;
}

internal void
Stack_Push(void)
{
    vptr *Marker = (vptr*)Context.Stack->Cursor; // ... [OldMarker]    [NewMarker]
    *Marker = Context.Stack->FirstMarker;        // ... [OldMarker] <- [NewMarker]
    Context.Stack->FirstMarker = Marker;         // ... [OldMarker] <- [NewMarker] <- [Header]
    Context.Stack->Cursor += sizeof(vptr);
}

internal vptr
Stack_Allocate(u64 Size)
{
    vptr Result = Context.Stack->Cursor;
    Context.Stack->Cursor += Size;
    return Result;
}

internal void
Stack_Pop(void)
{
    Context.Stack->FirstMarker = *Context.Stack->FirstMarker;
    Context.Stack->Cursor = (u08*)Context.Stack->FirstMarker + sizeof(vptr);
}