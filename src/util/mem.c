/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                         **
**  Author: Aria Seiler                                                    **
**                                                                         **
**  This program is in the public domain. There is no implied warranty,    **
**  so use it at your own risk.                                            **
**                                                                         **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifdef INCLUDE_HEADER

#define HEAP(type) heap_handle *

typedef vptr heap;
typedef struct heap_handle {
   u08 *Data;
   u64 Index    : 16;
   u64 Offset   : 46;
   u64 Free     :  1;
   u64 Anchored :  1;
   u32 Size;
   u16 PrevFree;
   u16 NextFree;
   u16 PrevUsed;
   u16 NextUsed;
   u16 PrevBlock;
   u16 NextBlock;
} heap_handle;

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
    u16 Data16 = ((u16)Data << 8) | Data;
    u32 Data32 = ((u32)Data16 << 16) | Data16;
    u64 Data64 = ((u64)Data32 << 32) | Data32;
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





internal heap *Heap_GetHeap(heap_handle *Handle) { return (heap*)(Handle-Handle->Index); }
internal heap_handle *Heap_GetHandle(vptr Data) { return *((heap_handle**)Data-1); }

internal heap *
Heap_Init(vptr MemBase,
          u64 Size)
{
    Assert(MemBase);
    Assert(Size > sizeof(heap_handle));
    Assert(Size - sizeof(heap_handle) < (1ULL<<46));
    
    heap_handle *NullUsedHandle = (heap_handle*)MemBase;
    NullUsedHandle->Data = (u08*)MemBase;
    NullUsedHandle->Size = sizeof(heap_handle);
    NullUsedHandle->Offset = Size - sizeof(heap_handle);
    NullUsedHandle->Index = 0;
    NullUsedHandle->PrevFree = 0;
    NullUsedHandle->NextFree = 0;
    NullUsedHandle->PrevUsed = 0;
    NullUsedHandle->NextUsed = 0;
    NullUsedHandle->PrevBlock = 0;
    NullUsedHandle->NextBlock = 0;
    NullUsedHandle->Anchored = TRUE;
    NullUsedHandle->Free = FALSE;
    
    return (heap*)MemBase;
}

internal void
Heap_Defragment(heap *Heap)
{
    Assert(FALSE); // Hasn't been debugged
    
    Assert(Heap);
    
    heap_handle *Handles = (heap_handle*)Heap;
    
    u64 Offset = 0;
    heap_handle *Block = Handles;
    do {
        Block = Handles + Block->PrevBlock;
        
        if(Block->Anchored) {
            Block->Offset += Offset;
            Offset = 0;
            continue;
        }
        
        Offset += Block->Offset;
        Block->Offset = 0;
        
        if(Offset) {
            Mem_Cpy(Block->Data+Offset, Block->Data, Block->Size);
            Block->Data += Offset;
        }
    } while(Block->Index);
}

internal void
Heap_AllocateBlock(heap *Heap,
                   heap_handle *Handle,
                   u32 Size)
{
    heap_handle *Handles = (vptr)Heap;
    heap_handle *PrevBlock = Handles + Handles[0].PrevBlock;
    while(PrevBlock->Index && PrevBlock->Offset < Size)
        PrevBlock = Handles+PrevBlock->PrevBlock;
    if(PrevBlock->Offset < Size) {
        Heap_Defragment(Heap);
        Assert(PrevBlock->Offset >= Size, "Not enough memory for new heap block");
    }
    Handle->Data = PrevBlock->Data + PrevBlock->Size + PrevBlock->Offset - Size;
    PrevBlock->Offset -= Size;
    Handle->PrevBlock = PrevBlock->Index;
    Handle->NextBlock = PrevBlock->NextBlock;
    Handles[Handle->NextBlock].PrevBlock = Handle->Index;
    Handles[Handle->PrevBlock].NextBlock = Handle->Index;
    Handle->Offset = 0;
    Handle->Size = Size;
}

internal void
Heap_FreeBlock(heap *Heap,
               heap_handle *Handle)
{
    heap_handle *Handles = (vptr)Heap;
    Handles[Handle->PrevBlock].Offset += Handle->Size + Handle->Offset;
    Handles[Handle->PrevBlock].NextBlock = Handle->NextBlock;
    Handles[Handle->NextBlock].PrevBlock = Handle->PrevBlock;
}

internal heap_handle *
_Heap_Allocate(heap *Heap,
               u32 Size,
               b08 Anchored)
{
    Assert(Heap);
    
    heap_handle *Handles = (heap_handle*)Heap;
    heap_handle *Handle = NULL;
    b08 Defragmented = FALSE;
    
    if(Anchored) Size += sizeof(heap_handle*);
    
    heap_handle *PrevUsed;
    if(Handles[0].NextFree == 0) {
        if(Handles[0].Offset < sizeof(heap_handle)) {
            Heap_Defragment(Heap);
            Defragmented = TRUE;
            Assert(Handles[0].Offset >= sizeof(heap_handle), "Not enough memory for new heap handle");
        }
        
        u16 HandleCount = (u16)(Handles[0].Size / sizeof(heap_handle));
        Handle = Handles + HandleCount;
        Handle->Index = HandleCount;
        
        Handles[0].Size += sizeof(heap_handle);
        Handles[0].Offset -= sizeof(heap_handle);
        
        PrevUsed = Handles + Handles[0].PrevUsed;
    } else {
        Handle = Handles + Handles[0].NextFree;
        Handles[Handle->PrevFree].NextFree = Handle->NextFree;
        Handles[Handle->NextFree].PrevFree = Handle->PrevFree;
        
        PrevUsed = Handle;
        while(PrevUsed->Index && PrevUsed->Free) PrevUsed--;
    }
    Handle->PrevUsed = PrevUsed->Index;
    Handle->NextUsed = PrevUsed->NextUsed;
    Handle->PrevFree = 0;
    Handle->NextFree = 0;
    Handles[Handle->PrevUsed].NextUsed = Handle->Index;
    Handles[Handle->NextUsed].PrevUsed = Handle->Index;
    Handle->Anchored = Anchored;
    Handle->Free = FALSE;
    
    Heap_AllocateBlock(Heap, Handle, Size);
    
    if(Anchored) *(heap_handle**)Handle->Data = Handle;
    
    return Handle;
}

internal heap_handle *Heap_Allocate (heap *Heap, u64 Size) { return _Heap_Allocate(Heap, Size, FALSE); }
internal vptr         Heap_AllocateA(heap *Heap, u64 Size) { return _Heap_Allocate(Heap, Size, TRUE)->Data + sizeof(heap_handle*); }

internal void
Heap_Resize(heap_handle *Handle,
            u32 NewSize)
{
    Assert(Handle);
    
    if(NewSize <= Handle->Size + Handle->Offset) {
        Handle->Offset += (s64)Handle->Size - (s64)NewSize;
        Handle->Size = NewSize;
    } else {
        u08 *PrevData = Handle->Data;
        u32 PrevSize = Handle->Size;
        heap *Heap = (heap*)(Handle - Handle->Index);
        Heap_FreeBlock(Heap, Handle);
        Heap_AllocateBlock(Heap, Handle, NewSize);
        Mem_Cpy(Handle->Data, PrevData, PrevSize);
    }
}

internal void
Heap_ResizeA(vptr *Data,
             u32 NewSize)
{
    heap_handle *Handle = Heap_GetHandle(*Data);
    Heap_Resize(Handle, NewSize);
    *Data = Handle->Data;
}

internal void
Heap_Free(heap_handle *Handle)
{
    Assert(Handle);
    heap_handle *Handles = Handle - Handle->Index;
    heap *Heap = (vptr)Handles;
    
    Heap_FreeBlock(Heap, Handle);
    
    Handles[Handle->NextUsed].PrevUsed = Handle->PrevUsed;
    Handles[Handle->PrevUsed].NextUsed = Handle->NextUsed;
    
    if(Handle->NextUsed == 0) {
        heap_handle *PrevFree = Handles + Handle->PrevUsed;
        while(PrevFree->Index && !PrevFree->Free) PrevFree--;
        Handles[0].PrevFree = PrevFree->Index;
        PrevFree->NextFree = 0;
        
        u16 Offset = Handle->PrevUsed + 1;
        u64 NewSize = Offset*sizeof(heap_handle);
        u64 DeltaSize = Handles[0].Size - NewSize;
        Handles[0].Size = NewSize;
        Handles[0].Offset += DeltaSize;
        Mem_Set(Handles+Offset, 0, DeltaSize);
    } else {
        heap_handle *PrevFree = Handle;
        while(PrevFree->Index && !PrevFree->Free) PrevFree--;
        
        Handle->Data = NULL;
        Handle->Offset = 0;
        Handle->Free = TRUE;
        Handle->Anchored = FALSE;
        Handle->Size = 0;
        Handle->PrevFree = PrevFree->Index;
        Handle->NextFree = PrevFree->NextFree;
        Handle->PrevUsed = 0;
        Handle->NextUsed = 0;
        Handle->PrevBlock = 0;
        Handle->NextBlock = 0;
        Handles[Handle->NextFree].PrevFree = Handle->Index;
        Handles[Handle->PrevFree].NextFree = Handle->Index;
    }
}

internal void Heap_FreeA(vptr Data) { Heap_Free(Heap_GetHandle(Data)); }

#endif