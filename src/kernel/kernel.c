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
   ST_InvalidParameter,
   ST_NotFound,
} thunderos_status;

typedef enum thunderos_flags {
   HW_HasSerial = 0x01
} thunderos_flags;

typedef enum cr0_flag {
   CR0_ProtectionEnable   = 0x00000001,
   CR0_MonitorCoprocessor = 0x00000002,
   CR0_Emulation          = 0x00000004,
   CR0_TaskSwitched       = 0x00000008,
   CR0_ExtensionType      = 0x00000010,
   CR0_NumericError       = 0x00000020,
   CR0_WriteProtect       = 0x00010000,
   CR0_AlignmentMask      = 0x00040000,
   CR0_NotWriteThrough    = 0x20000000,
   CR0_CacheDisable       = 0x40000000,
   CR0_Paging             = 0x80000000,
} cr0_flag;

typedef enum cr3_flag {
   CR3_PageWriteThrough = 0x00000008,
   CR3_PageCacheDisable = 0x00000010,
} cr3_flag;

typedef enum cr4_flag {
   CR4_Virtual8086Extensions           = 0x00000001,
   CR4_ProtectedVirtualInterrupts      = 0x00000002,
   CR4_TimeStampDisable                = 0x00000004,
   CR4_DebuggingExtensions             = 0x00000008,
   CR4_PageSizeExtensions              = 0x00000010,
   CR4_PhysicalAddressExtension        = 0x00000020,
   CR4_MachineCheckEnable              = 0x00000040,
   CR4_PageGlobalEnable                = 0x00000080,
   CR4_PerformanceCounter              = 0x00000100,
   CR4_FXSAVEAndFXRSTOR                = 0x00000200,
   CR4_SIMDFloatExceptions             = 0x00000400,
   CR4_UserModeInstructionPreventation = 0x00000800,
   CR4_57BitLinearAddress              = 0x00002000,
   CR4_VMXEnable                       = 0x00004000,
   CR4_SMXEnable                       = 0x00010000,
   CR4_FSGSBASEEnable                  = 0x00020000,
   CR4_PCIDEnable                      = 0x00040000,
   CR4_XSAVEAndExtendedStates          = 0x00080000,
   CR4_KeyLocker                       = 0x00100000,
   CR4_SMEPEnable                      = 0x00200000,
   CR4_SMAPEnable                      = 0x00400000,
   CR4_ControlFlowEnforcement          = 0x00800000,
   CR4_SupervisorProtectionKeys        = 0x01000000,
} cr4_flag;

#define INCLUDE_HEADER
   #include <util/math.c>
   #include <util/vector.c>
   #include <util/scalar.c>
   
   #include <kernel/efi.h>
   
   #include <drivers/serial.c>
   #include <drivers/descriptors.c>
   #include <drivers/acpi.c>
   #include <drivers/mem.c>
   #include <drivers/pci.c>
   
   #include <render/font.c>
   #include <render/terminal.c>
#undef INCLUDE_HEADER

#undef Assert
#define Assert(...) UNUSED(__VA_ARGS__)

extern u08  PortIn08(u16 Address);
extern u32  PortIn32(u16 Address);
extern void PortOut08(u16 Address, u08 Data);
extern void PortOut32(u16 Address, u32 Data);
extern void SetGDTR(vptr GDT, u16 Size);
extern void SetIDTR(idt *IDT, u16 Size);
extern u64  GetMSR(u32 Base);
extern void SetMSR(u32 Base, u64 Value);
extern u64  GetCR0(void);
extern u64  GetCR3(void);
extern u64  GetCR4(void);
extern void SetCR3(u64);
extern void DisableInterrupts(void);
extern void EnableInterrupts(void);

global palloc_dir_map *DirMap;

internal void
KernelError(c08 *File, u32 Line, c08 *Expression)
{
    
}

#define INCLUDE_SOURCE
   #include <util/vector.c>
   #include <util/mem.c>
   #include <util/str.c>
   
   #include <drivers/serial.c>
   #include <drivers/interrupts.c>
   #include <drivers/descriptors.c>
   #include <drivers/acpi.c>
   #include <drivers/mem.c>
   #include <drivers/pci.c>
   
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
             efi_pci_root_bridge_io_protocol *PRBIP,
             efi_simple_file_system_protocol *SFSP,
             efi_memory_descriptor *MemoryMap,
             u64 MemoryMapDescriptorSize,
             u32 MemoryMapDescriptorCount,
             vptr PAllocPages)
{
   u32 Status;
   
   DisableInterrupts();
   
   
   u64 CR0 = GetCR0();
   u64 CR3 = GetCR3();
   u64 CR4 = GetCR4();
   
   //TODO: Guarantee the existance of a map set at 0
   //TODO: Guarantee that CR3 is recursively mapped.
   if(CR4 & CR4_57BitLinearAddress) {
      // TODO
   } else {
      page_map_lvl4 *Lvl4  = (vptr)(CR3 & 0xFFFFFFFFFFFFF000);
      page_map_lvl3 *Lvl3  = (vptr)(Lvl4->Entries[0] & 0x000FFFFFFFFFF000);
      page_directory *Lvl2 = (vptr)(Lvl3->Entries[0] & 0x000FFFFFFFFFF000);
      page_table *Lvl1     = (vptr)(Lvl2->Entries[0] & 0x000FFFFFFFFFF000);
      vptr Page            = (vptr)(Lvl1->Entries[0] & 0x000FFFFFFFFFF000);
      Page = NULL;
   }
   
   // Initialize as if all pages are being used
   DirMap = (vptr)PAllocPages;
   Mem_Set(DirMap, -1, 1024);
   Mem_Set(DirMap->Dirs, 0, sizeof(palloc_dir*) * 256);
   
   // 'Free' the memory that's available
   for(u32 I = 0; I < MemoryDescriptorCount; I++) {
      efi_memory_descriptor *Descriptor = (vptr)((u08*)MemoryMap + MemoryMapDescriptorSize*I);
      
      switch(Descriptor->Type) {
         /*
         case EFI_MemoryType_Reserved:
         case EFI_MemoryType_LoaderData:
         case EFI_MemoryType_RuntimeServicesCode:
         case EFI_MemoryType_RuntimeServicesData:
         case EFI_MemoryType_UnusableMemory:
         case EFI_MemoryType_ACPIReclaim:
         case EFI_MemoryType_ACPIMemoryNVS:
         case EFI_MemoryType_MappedIO:
         case EFI_MemoryType_MappedIOPortSpace:
         case EFI_MemoryType_PalCode:
         case EFI_MemoryType_Unaccepted:
         */
         case EFI_MemoryType_LoaderCode:
         case EFI_MemoryType_BootServicesCode:
         case EFI_MemoryType_BootServicesData:
         case EFI_MemoryType_ConventionalMemory:
         case EFI_MemoryType_Persistent: {
            FreePhysicalPageRange(Descriptor->PhysicalStart, Descriptor->PageCount);
            FreeVirtualPageRange(Descriptor->VirtualStart, Descriptor->PageCount);
         } break;
      }
   }
   
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
   
   //TODO: Reclaim the ACPI data
   
   idt IDT;
   IDT_Init(&IDT);
   
   //TODO: Pluggable drivers?
   //TODO: Handle when certain devices are unavailable
   
   pci PCI;
   if(PRBIP) {
      PCI = PCI_Init();
      // PCI_GetDeviceMemory(PCI_Serial, PCI_Serial_USB, PCI_Serial_USB_XHCI);
   }
   
   //TODO: Definitely need a better PCI system
   if(PCI.XHCI & 0x80000000) {
      u08 B = (PCI.XHCI >> 16) & 0xFF;
      u08 D = (PCI.XHCI >>  8) & 0xFF;
      u08 F = (PCI.XHCI >>  0) & 0xFF;
      u64 BAR0 = PCI_Read32(B, D, F, OFFSETOF(pci_header, Type0.BaseAddress0));
      u64 BAR1 = PCI_Read32(B, D, F, OFFSETOF(pci_header, Type0.BaseAddress1));
      u64 Address = (BAR1<<32) | BAR0;
      
      // PCI_EnableMSI(B, D, F);
   }
   
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
   
   Serial_Write(SerialPort, "CR0: ");
   Serial_Write(SerialPort, U64_ToStr(Buffer, CR0, 16));
   Serial_Write(SerialPort, "\r\nCR3: ");
   Serial_Write(SerialPort, U64_ToStr(Buffer, CR3, 16));
   Serial_Write(SerialPort, "\r\nCR4: ");
   Serial_Write(SerialPort, U64_ToStr(Buffer, CR4, 16));
   Serial_Write(SerialPort, "\r\n");
   
   // u64 *Ptr = (u64*)0x0000FFFFFFFFFFFF;
   // *Ptr = 5;
   
   while(1);
   
   return EFI_Status_Success;
}