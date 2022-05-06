/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                         **
**  Author: Aria Seiler                                                    **
**                                                                         **
**  This program is in the public domain. There is no implied warranty,    **
**  so use it at your own risk.                                            **
**                                                                         **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifdef INCLUDE_HEADER

// typedef struct stack {
//     u64 Size;
//     vptr *FirstMarker;
//     u08 *Cursor;
// } stack;

#endif

#ifdef INCLUDE_SOURCE

internal vptr
Mem_Set(vptr Dest,
        u08 Data,
        u64 Size)
{
    u08 *Dest08 = (u08*)Dest;
    
    u64 ToAlign = (8 - ((u64)Dest & 7)) & 8;
    Size -= ToAlign;
    while(ToAlign) {
        *Dest08++ = Data;
        ToAlign--;
    }
    
    // TODO: Check for SSE
    u64 *Dest64 = (u64*)Dest;
    u64 Data64 = ((u64)Data<<56)|((u64)Data<<48)|((u64)Data<<40)|((u64)Data<<32)|((u64)Data<<24)|((u64)Data<<16)|((u64)Data<<8)|Data;
    while(Size >= 8) {
        *Dest64++ = Data64;
        Size -= 8;
    }
    
    Dest08 = (u08*)Dest64;
    while(Size) {
        *Dest08++ = Data;
        Size--;
    }
    
    return Dest;
}

internal vptr
Mem_Cpy(vptr Dest,
        vptr Src,
        u64 Size)
{
    u08 *D08 = (u08*)Dest;
    u08 *S08 = (u08*)Src;
    while(Size)
    {
        *D08++ = *S08++;
        Size--;
    }
    
    return Dest;
}

internal vptr
Mem_Tile(vptr Dest,
         u64 DestSize,
         vptr Tile,
         u64 TileSize)
{
    u32 TileCount = DestSize / TileSize;
    u08 *WriteCursor = (u08*)Dest;
    for(u32 Index = 0; Index < TileCount; ++Index)
    {
        Mem_Cpy(WriteCursor, Tile, TileSize);
        WriteCursor += TileSize;
    }
    
    return Dest;
}

internal s32
Mem_Cmp(vptr A,
        vptr B,
        u64 Size)
{
    u64 I = 0;
    u08 *CA = (u08*)A;
    u08 *CB = (u08*)B;
    while(I < Size && *CA == *CB)
    {
        CA++;
        CB++;
        I++;
    }
    
    if(I == Size) return EQUAL;
    if(CA < CB)   return GREATER;
    return LESS;
}

internal u32
Mem_BytesUntil(u08 Byte, vptr Data)
{
    u08 *Data08 = (u08*)Data;
    u32 Length = 0;
    
    while(*Data08 != Byte)
    {
        Data08++;
        Length++;
    }
    
    return Length;
}




// typedef struct page_buddy {
//     u08 BitCount;
//     u08 *Bitmap;
// } page_buddy;

// typedef struct page_allocator {
//     page_buddy Buddies[8];
// } page_allocator;

// void PageAlloc(u32 PageCount)
// {
//     if(PageCount == 0) return;
    
//     s32 BuddyIndex;
//     if(PageCount == 1) BuddyIndex = 0;
//     else {
//         asm("bsr %1, %0"
//             : "=r" (BuddyIndex)
//             : "rm" (PageCount-1)
//         );
//         BuddyIndex++;
//     }
    
//     u32 I = 0;
//     page_buddy *Buddy = Context.PageAllocator.Buddies+BuddyIndex;
//     while(I < Buddy->BitCount) {
//         b08 Mask = 1 << (I%8);
//         b08 *Byte = Buddy->Bitmap + I/8;
//         if(!(*Byte & Mask)) {
//             *Byte |= Mask;
//             break;
//         }
//         I++;
//     }
//     Assert(I < Buddy->BitCount);
    
//     while(BuddyIndex > 0) {
//         BuddyIndex--;
//         Buddy = Context.PageAllocator.Buddies+BuddyIndex;
//         b08 *Byte = Buddy->Bitmap + I/4;
//         b08 Mask = 1 << ((I%4)*2);
        
        
        
//     }
// }









// internal stack *
// Linear_Init(vptr Mem,
//            u64 Size)
// {
//     stack *Result = Mem;
//     Result->Size = Size - sizeof(stack);
//     Result->FirstMarker = (vptr*)&Result->FirstMarker;
//     Result->Cursor = (u08*)Mem + sizeof(stack) + sizeof(vptr);
//     return Result;
// }

// internal void
// Linear_Push(void)
// {
//     vptr *Marker = (vptr*)Context.Stack->Cursor; // ... [OldMarker]    [NewMarker]
//     *Marker = Context.Stack->FirstMarker;        // ... [OldMarker] <- [NewMarker]
//     Context.Stack->FirstMarker = Marker;         // ... [OldMarker] <- [NewMarker] <- [Header]
//     Context.Stack->Cursor += sizeof(vptr);
// }

// internal vptr
// Linear_Allocate(u64 Size)
// {
//     vptr Result = Context.Stack->Cursor;
//     Context.Stack->Cursor += Size;
//     return Result;
// }

// internal void
// Linear_Pop(void)
// {
//     Context.Stack->FirstMarker = *Context.Stack->FirstMarker;
//     Context.Stack->Cursor = (u08*)Context.Stack->FirstMarker + sizeof(vptr);
// }

#endif