/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                         **
**  Author: Aria Seiler                                                    **
**                                                                         **
**  This program is in the public domain. There is no implied warranty,    **
**  so use it at your own risk.                                            **
**                                                                         **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <shared.h>
#include <kernel/efi.h>

extern void DisableInterrupts(void);
extern void EnableInterrupts(void);
extern u08  PortIn08(u16 Address);
// extern u16  PortIn16(u16 Address);
// extern u32  PortIn32(u16 Address);
extern void PortOut08(u16 Address, u08 Data);
// extern void PortOut16(u16 Address, u16 Data);
// extern void PortOut32(u16 Address, u32 Data);
extern void WriteGDTR(gdt *GDT, u16 Size);
// extern void WriteIDTR(idt *IDT, u16 Size);

// #define PortWait PortOut08(0x80, 0);

typedef enum thunderos_status {
   ST_Success,
} thunderos_status;

typedef enum thunderos_flags {
   HW_HasSerial
} thunderos_flags;

#include <drivers/serial.c>
#include <drivers/gdt.c>

#if 0

typedef struct context {
    vptr (API *Allocate) (u64 Size);
    
    struct stack *Stack;
    struct terminal *Terminal;
    
    u32 *Framebuffer;
    u64 FramebufferSize;
    
    struct context *PrevContext;
} context;
// global u64 PageDirPtrTbl[4] __attribute__((align(32)));
// global u64 PageDir[512] __attribute__((align(4096)));
// global u32 PageTbl[1024] __attribute__((align(4096)));

typedef enum type {
    Type_C08p,
    Type_Str,
    Type_U08,
    Type_U16,
    Type_U32,
    Type_U64,
} type;

global context Context;

internal void KernelError(c08 *File, u32 Line, c08 *Expression);
internal void Convert(vptr Out, type TypeOut, vptr In, type TypeIn);

typedef struct __attribute__((packed)) bitmap_header {
    c08 Signature[2];
    u32 FileSize;
    u32 Reserved;
    u32 DataOffset;
    u32 Size;
    u32 Width;
    u32 Height;
    u16 Planes;
    u16 BitsPerPixel;
    u32 Compression;
    u32 ImageSize;
    u32 XPixelsPerM;
    u32 YPixelsPerM;
    u32 ColorsUsed;
    u32 ImportantColors;
} bitmap_header;

typedef struct gdt_segment_descriptor {
    u16 Limit;
    u16 BaseP1;
    u08 BaseP2;
    u08 AccessByte;
    u08 Attributes;
    u08 BaseP3;
} gdt_segment_descriptor;

typedef struct gdt {
    gdt_segment_descriptor Entries[8];
} gdt;

typedef enum interrupt_ids {
    Interrupt_Test = 0x00,
} interrupt_ids;

typedef enum idt_attribute_flags {
    IDT_Gate_Interrupt = 0b1110,
    IDT_Gate_Trap      = 0b1111,
    
    IDT_DPL_Ring0 = 0b00,
    IDT_DPL_Ring1 = 0b01,
    IDT_DPL_Ring2 = 0b10,
    IDT_DPL_Ring3 = 0b11,
} idt_attribute_flags;

typedef struct idt_gate_descriptor {
    u16 OffsetP1;
    u16 SegmentSelector;
    u16 Attributes;
    u16 OffsetP2;
    u32 OffsetP3;
    u32 _Reserved1;
} idt_gate_descriptor;

typedef struct idt {
    idt_gate_descriptor Entries[256];
} idt;

#include <kernel/efi.h>
#include <util/intrin.h>
#include <util/mem.c>
#include <util/vector.c>
#include <util/str.c>
#include <drivers/acpi.c>
#include <drivers/ps2.c>
#include <render/font.c>
#include <render/software.c>
#include <render/terminal.c>

internal void
Convert(vptr Out,
        type TypeOut,
        vptr In,
        type TypeIn)
{
    b08 IsUInt = FALSE;
    u64 UIntValue = 0;
    
    switch(TypeIn) {
        case Type_C08p: {
            c08 *C08p = *(c08**)In;
            u32 Length = Mem_BytesUntil(0, C08p);
            
            switch(TypeOut) {
                case Type_Str: {
                    str Result;
                    Result.Length = Mem_BytesUntil(0, C08p);
                    Result.Capacity = Result.Length;
                    Result.Data = Context.Allocate(Result.Length * sizeof(c16));
                    for(u32 I = 0; I < Length; ++I)
                        Result.Data[I] = (c16)C08p[I];
                    
                    *(str*)Out = Result;
                } break;
            }
        } break;
        
        case Type_U08: {
            UIntValue = (u64)*(u08*)In;
            IsUInt = TRUE;
        } break;
        case Type_U16: {
            UIntValue = (u64)*(u16*)In;
            IsUInt = TRUE;
        } break;
        case Type_U32: {
            UIntValue = (u64)*(u32*)In;
            IsUInt = TRUE;
        } break;
        case Type_U64: {
            UIntValue = *(u64*)In;
            IsUInt = TRUE;
        } break;
        
        NO_DEFAULT;
    }
    
    if(IsUInt) {
        switch(TypeOut) {
            case Type_Str: {
                str Result;
                Result.Capacity = 20; // Max length of unsigned 64-bit base 10
                Result.Data = Context.Allocate(Result.Capacity);
                
                u32 Index = Result.Capacity;
                do {
                    Index--;
                    Result.Data[Index] = (UIntValue % 10) + L'0';
                    UIntValue /= 10;
                } while(UIntValue > 0);
                
                Result.Length = Result.Capacity - Index;
                Result.Data += Index;
                
                *(str*)Out = Result;
            } break;
            
            NO_DEFAULT;
        }
    }
}

// TODO: Remove dependencies to everything
internal void
KernelError(c08 *File,
            u32 Line,
            c08 *Expression)
{
    if(Context.Framebuffer != NULL) {
        if(Context.Terminal != NULL) {
            Mem_Set(Context.Framebuffer, 0, Context.FramebufferSize);
            
            c08 Buffer[32];
            u32 Index = sizeof(Buffer);
            u32 Value = Line;
            Buffer[--Index] = 0;
            do {
                Buffer[--Index] = (Value % 10) + '0';
                Value /= 10;
            } while(Value > 0);
            
            WriteToTerminal(Context.Terminal, "\n\n", 0);
            WriteToTerminal(Context.Terminal, File, 0);
            WriteToTerminal(Context.Terminal, ":", 0);
            WriteToTerminal(Context.Terminal, Buffer+Index, 0);
            WriteToTerminal(Context.Terminal, ": ERROR: (", 0);
            WriteToTerminal(Context.Terminal, Expression, 0);
            WriteToTerminal(Context.Terminal, ") was FALSE\n", 0);
            DrawTerminal(Context.Framebuffer, Context.Terminal);
        }
    }
    
    while(TRUE)
        asm volatile ("pause");
}
#endif

#undef Assert
#define Assert(...)

internal void
InitGOP(efi_graphics_output_protocol *GOP)
{
    u64 SizeOfGOPInfo;
    efi_graphics_output_mode_information *Info;
    Status = GOP->QueryMode(GOP, GOP->Mode->Mode, &SizeOfGOPInfo, &Info);
    Assert(Status == EFI_Status_Success);
    // Context.FramebufferSize = GOP->Mode->FrameBufferSize;
    // Context.Framebuffer = (u32*)GOP->Mode->FrameBufferBase;
}

external u32
Kernel_Entry(efi_graphics_output_protocol *GOP)
{
   u32 Status;
   u64 Flags = 0;
   
   DisableInterrupts();
   
   InitGOP(GOP);
   
   u16 SerialPort;
   Status = InitSerial(38400, &SerialPort);
   if(Status == ST_Success) {
      Flags |= HW_HasSerial;
   }
   
   EnableInterrupts();
   
   return 0;
}
