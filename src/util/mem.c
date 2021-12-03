/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                         **
**  Author: Aria Seiler                                                    **
**                                                                         **
**  This program is in the public domain. There is no implied warranty,    **
**  so use it at your own risk.                                            **
**                                                                         **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */



typedef struct stack {
    u64 Size;
    u08 *Cursor;
    u64 *FirstMarker;
} stack;

internal stack *
Stack_Init(vptr Mem,
           u64 Size)
{
    stack *Result = Mem;
    Result->Size = Size - sizeof(stack);
    Result->Cursor = (u08*)Mem + sizeof(stack);
    Result->FirstMarker = Result->Cursor;
    return Result;
}

internal void
Stack_Push(void)
{
    *(u64*)Context.Stack->Cursor = *Context.Stack->FirstMarker;
    Context.Stack->
    Context.Stack->Cursor += sizeof(u64);
}