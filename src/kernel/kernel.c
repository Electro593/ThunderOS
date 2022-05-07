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

global struct {
    u64 Flags;
    u16 SerialPort;
} Global;

typedef enum thunderos_status {
   ST_Success,
   ST_NotSupported,
} thunderos_status;

typedef enum thunderos_flags {
   HW_HasSerial = 0x01
} thunderos_flags;

#define INCLUDE_HEADER
    #include <util/vector.c>
    #include <util/mem.c>
    
    #include <kernel/efi.h>
    
    #include <drivers/serial.c>
    #include <drivers/descriptors.c>
    #include <drivers/acpi.c>
    #include <drivers/mem.c>
    
    #include <render/font.c>
    #include <render/terminal.c>
#undef INCLUDE_HEADER

#undef Assert
#define Assert(...) UNUSED(__VA_ARGS__)

extern u08  PortIn08(u16 Address);
extern void PortOut08(u16 Address, u08 Data);
extern void SetGDTR(vptr GDT, u16 Size);
extern void SetIDTR(idt *IDT, u16 Size);
extern u64  GetMSR(u32 Base);
extern void SetMSR(u32 Base, u64 Value);
extern u64  GetCR3(void);
extern void DisableInterrupts(void);
extern void EnableInterrupts(void);

#define INCLUDE_SOURCE
    #include <util/vector.c>
    #include <util/mem.c>
    #include <util/str.c>
    
    #include <drivers/serial.c>
    #include <drivers/interrupts.c>
    #include <drivers/descriptors.c>
    #include <drivers/acpi.c>
    #include <drivers/mem.c>
    
    #include <render/terminal.c>
#undef INCLUDE_SOURCE

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

#include <util/intrin.h>
#include <util/vector.c>
#include <util/str.c>
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

internal void
InitGOP(efi_graphics_output_protocol *GOP)
{
    u64 SizeOfGOPInfo;
    efi_graphics_output_mode_information *Info;
    efi_status Status = GOP->QueryMode(GOP, GOP->Mode->Mode, &SizeOfGOPInfo, &Info);
    Assert(Status == EFI_Status_Success);
    // Context.FramebufferSize = GOP->Mode->FrameBufferSize;
    // Context.Framebuffer = (u32*)GOP->Mode->FrameBufferBase;
}

external u32
Kernel_Entry(rsdp *RSDP,
             efi_graphics_output_protocol *GOP,
             efi_simple_file_system_protocol *SFSP,
             efi_memory_descriptor *MemoryMap,
             u64 MemoryMapDescriptorSize,
             u32 MemoryMapDescriptorCount)
{
    u32 Status;
    
    DisableInterrupts();
    
    InitGOP(GOP);
    
    Status = Serial_Init(38400, &Global.SerialPort);
    u16 SerialPort = Global.SerialPort;
    if(Status == ST_Success) {
       Global.Flags |= HW_HasSerial;
    }
    
    gdt GDT;
    tss TSS;
    u08 RingStacks[3][4096];
    u08 ISTStacks[7][4096];
    GDT_Init(&GDT, &TSS, (vptr*)RingStacks, (vptr*)ISTStacks);
    
    acpi ACPI = InitACPI(RSDP);
    InitAPIC(ACPI);
    
    idt IDT;
    IDT_Init(&IDT);
    
    EnableInterrupts();
    
    c08 Buffer[64];
    
    Serial_Write(SerialPort, "Hello! Testing the serial output\r\n");
    
    for(u32 I = 0; I < MemoryMapDescriptorCount; I++) {
        efi_memory_descriptor *Descriptor = (vptr)((u08*)MemoryMap + MemoryMapDescriptorSize*I);
        
        Serial_Write(SerialPort, U64_ToStr(Buffer, Descriptor->Type, 16));
        Serial_Write(SerialPort, "\t");
        Serial_Write(SerialPort, U64_ToStr(Buffer, (u64)Descriptor->PhysicalStart, 16));
        Serial_Write(SerialPort, "\t");
        Serial_Write(SerialPort, U64_ToStr(Buffer, (u64)Descriptor->VirtualStart, 16));
        Serial_Write(SerialPort, "\t");
        Serial_Write(SerialPort, U64_ToStr(Buffer, Descriptor->PageCount, 10));
        Serial_Write(SerialPort, "\r\n");
    }
    
    // page_map_lvl4 PageMapLvl4 = *(vptr)GetCR3();
    
    
    Serial_Write(SerialPort, U64_ToStr(Buffer, cr3, 16));
    
    
    while(1);
    
    return EFI_Status_Success;
}