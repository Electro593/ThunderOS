/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                         **
**  Author: Aria Seiler                                                    **
**                                                                         **
**  This program is in the public domain. There is no implied warranty,    **
**  so use it at your own risk.                                            **
**                                                                         **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <shared.h>
#include <kernel/elf.h>

// CREDIT: POSIX-UEFI
asm (
"        .align 4                  \n"
"        .globl _start             \n"
"                                  \n"
"_start: lea ImageBase(%rip), %rdi \n"
"        lea _DYNAMIC(%rip), %rsi  \n"
"        sub $8, %rsp              \n"
"        call EFI_Entry            \n"
"        ret                       \n"
"                                  \n"
"  /* Fake a relocation record */  \n"
"        .data                     \n"
"dummy:  .long 0                   \n"
"        .section .reloc, \"a\"    \n"
"                                  \n"
"label1: .long   dummy-label1      \n"
"        .long   10                \n"
"        .word 0                   \n"
"                                  \n"
"        .text                     \n"
);

// asm (
// "        .globl setjmp         \n"
// "                              \n"
// "setjmp: pop  %rsi             \n"
// "        movq %rbx, 0x00(%rdi) \n"
// "        movq %rsp, 0x08(%rdi) \n"
// "        push %rsi             \n"
// "        movq %rbp, 0x10(%rdi) \n"
// "        movq %r12, 0x18(%rdi) \n"
// "        movq %r13, 0x20(%rdi) \n"
// "        movq %r14, 0x28(%rdi) \n"
// "        movq %r15, 0x30(%rdi) \n"
// "        movq %rsi, 0x38(%rdi) \n"
// "        xor  %rax, %rax       \n"
// "        ret                   \n"
// );

// asm (
// "         .globl longjmp        \n"
// "                               \n"
// "longjmp: movl %esi, %eax       \n"
// "         movq 0x00(%rdi), %rbx \n"
// "         movq 0x08(%rdi), %rsp \n"
// "         movq 0x10(%rdi), %rbp \n"
// "         movq 0x18(%rdi), %r12 \n"
// "         movq 0x20(%rdi), %r13 \n"
// "         movq 0x28(%rdi), %r14 \n"
// "         movq 0x30(%rdi), %r15 \n"
// "         xor %rdx, %rdx        \n"
// "         mov $1, %rcx          \n"
// "         cmp %rax, %rdx        \n"
// "         cmove %rcx, %rax      \n"
// "         jmp *0x38(%rdi)       \n"
// );

#define INCLUDE_HEADER
   #include <kernel/efi.h>
   #include <drivers/acpi.c>
#undef INCLUDE_HEADER

typedef u32
(*kernel_entry)(rsdp *RSDP,
                efi_graphics_output_protocol *GOP,
                efi_pci_root_bridge_io_protocol *PRBIP,
                efi_simple_file_system_protocol *SFSP,
                efi_memory_descriptor *MemoryMap,
                u64 MemoryMapDescriptorSize,
                u32 MemoryMapDescriptorCount);

internal void
U64_ToStr(c16 *Buffer, u64 N, u32 Radix) {
   persist c08 Chars[64] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-_";
   
   u64 M = N;
   u32 Len = 1;
   while(M /= Radix) Len++;
   
   if(Radix == 2) {
      Buffer[0] = '0';
      Buffer[1] = 'b';
      Len += 2;
   } else if(Radix == 8) {
      Buffer[0] = '0';
      Len += 1;
   } else if(Radix == 16) {
      Buffer[0] = '0';
      Buffer[1] = 'x';
      Len += 2;
   }
   
   Buffer[Len--] = 0;
   
   do {
      u16 Digit = N % Radix;
      Buffer[Len--] = (c16)Chars[Digit];
   } while(N /= Radix);
}

internal s08
_strncmp(c08 *A, c08 *B, u32 N) {
   while(N && *A == *B) N--, A++, B++;
   if(N == 0) return 0;
   if(*A < *B) return -1;
   return 1;
}

internal s08
_strcmp(c08 *A, c08 *B) {
   while(*A && *B && *A == *B) A++, B++;
   if(*A == *B) return 0;
   if(*A < *B) return -1;
   return 1;
}

external efi_status
EFI_Entry(u64 LoadBase,
          elf64_dynamic *Dynamics,
          efi_system_table *SystemTable,
          efi_handle ImageHandle)
{
   UNUSED(LoadBase);
   UNUSED(Dynamics);
   
   efi_status Status;
   efi_boot_services *BootServices = SystemTable->BootServices;
   
   #define ASSERT(Expression, Message) \
      do { \
         if(!(Expression)) { \
            SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, Message); \
            Status = EFI_Status_Aborted; \
            goto _error; \
         } \
      } while(0)
   #define SASSERT(DesiredStatus, Message) \
      ASSERT(DesiredStatus == Status, Message)
   
   SystemTable->ConsoleOut->Reset(SystemTable->ConsoleOut, FALSE);
   
   efi_loaded_image_protocol *LoadedImage = NULL;
   Status = BootServices->HandleProtocol(ImageHandle, &EFI_GUID_LOADED_IMAGE_PROTOCOL, (vptr*)&LoadedImage);
   SASSERT(EFI_Status_Success, u"ERROR: Could not load LoadedImage Protocol\r\n");
   
   c16 Buffer[128];
   U64_ToStr(Buffer, (u64)LoadedImage->ImageBase, 16);
   SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut,
      u"NOTE: Loader image base is at ");
   SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut,
      Buffer);
   SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut,
      u"\r\n");
   
   //
   // Load the kernel
   //
   efi_simple_file_system_protocol *SFSP;
   Status = BootServices->HandleProtocol(LoadedImage->DeviceHandle, &EFI_GUID_SIMPLE_FILE_SYSTEM_PROTOCOL, (vptr)&SFSP);
   SASSERT(EFI_Status_Success, u"ERROR: Could not load SimpleFileSystem Protocol\r\n");
   
   efi_file_protocol *Volume = 0;
   Status = SFSP->OpenVolume(SFSP, &Volume);
   SASSERT(EFI_Status_Success, u"ERROR: Could not open EFI volume\r\n");
   
   efi_file_protocol *FileHandle = 0;
   Status = Volume->Open(Volume, &FileHandle, L"\\kernel", EFI_FileMode_Read, 0);
   SASSERT(EFI_Status_Success, u"ERROR: Could not open kernel file\r\n");
   
   u64 FileInfoSize = 0;
   Status = FileHandle->GetInfo(FileHandle, &EFI_GUID_FILE_INFO, &FileInfoSize, NULL);
   SASSERT(EFI_Status_BufferTooSmall, u"ERROR: Could not get kernel file size\r\n");
   
   efi_file_info *FileInfo = 0;
   Status = BootServices->AllocatePool(EFI_MemoryType_LoaderData, FileInfoSize, (vptr*)&FileInfo);
   SASSERT(EFI_Status_Success, u"ERROR: Could not allocate kernel file info\r\n");
   Status = FileHandle->GetInfo(FileHandle, &EFI_GUID_FILE_INFO, &FileInfoSize, FileInfo);
   SASSERT(EFI_Status_Success, u"ERROR: Could not get kernel file info\r\n");
   u64 FileSize = FileInfo->FileSize;
   Status = BootServices->FreePool(FileInfo);
   SASSERT(EFI_Status_Success, u"ERROR: Could not free kernel file info\r\n");
   
   u08 *FileData = 0;
   Status = BootServices->AllocatePool(EFI_MemoryType_LoaderData, FileSize, (vptr*)&FileData);
   SASSERT(EFI_Status_Success, u"ERROR: Could not allocate kernel file\r\n");
   Status = FileHandle->Read(FileHandle, &FileSize, FileData);
   SASSERT(EFI_Status_Success, u"ERROR: Could not read kernel file\r\n");
   
   Status = FileHandle->Close(FileHandle);
   SASSERT(EFI_Status_Success, u"ERROR: Could not close kernel file\r\n");
   
   elf64_header *ELFHeader = (vptr)FileData;
   ASSERT(ELFHeader->ID[0] == 0x7F && ELFHeader->ID[1] == 'E' && ELFHeader->ID[2] == 'L' && ELFHeader->ID[3] == 'F', u"Invalid kernel ELF header magic number!\r\n");
   ASSERT(ELFHeader->ID[4] == 1 || ELFHeader->ID[4] == 2, u"Invalid kernel ELF header word size!\r\n");
   ASSERT(ELFHeader->ID[4] == 2, u"Kernel ELF must be 64-bit!\r\n");
   ASSERT(ELFHeader->ID[5] == 1 || ELFHeader->ID[5] == 2, u"Invalid kernel ELF header endianness value!\r\n");
   ASSERT(ELFHeader->ID[5] == 1, u"Kernel ELF must be little-endian!\r\n");
   ASSERT(ELFHeader->ID[6] == 1, u"Invalid kernel ELF header ID version!\r\n");
   ASSERT(ELFHeader->ID[7] <= 0x12, u"Invalid kernel ELF header target ABI!\r\n");
   ASSERT(ELFHeader->Type <= 0x12, u"Invalid kernel ELF type!\r\n");
   ASSERT(ELFHeader->Version == 1, u"Invalid kernel ELF header version!\r\n");
   
   u08 *ProgramHeaderBase = (vptr)(FileData + ELFHeader->ProgramHeaderOffset);
   u08 *SectionHeaderBase = (vptr)(FileData + ELFHeader->SectionHeaderOffset);
   
   ASSERT(ELFHeader->StringTableIndex != 0, u"ELF string table index must not be null!\r\n");
   elf64_section_header *StringTableHeader = (vptr)(SectionHeaderBase + ELFHeader->StringTableIndex*ELFHeader->SectionHeaderSize);
   c08 *StringTable = (vptr)(FileData + StringTableHeader->Offset);
   
   u64 KernelEntryAddress;
   
   switch(ELFHeader->Type) {
      case ELF_HeaderType_Rel: {
         vptr *SectionBases;
         Status = BootServices->AllocatePool(EFI_MemoryType_LoaderData, sizeof(u08*) * ELFHeader->SectionHeaderCount, (vptr*)&SectionBases);
         SASSERT(EFI_Status_Success, u"ERROR: Could not allocate memory for section bases\r\n");
         
         u64 BaseAddress = 0x1000;
         
         u64 PrevTextAlign = 1;
         u64 PrevRODataAlign = 1;
         u64 PrevDataAlign = 1;
         u64 PrevBSSAlign = 1;
         u64 TextSize = 0;
         u64 RODataSize = 0;
         u64 DataSize = 0;
         u64 BSSSize = 0;
         for(u32 I = 0; I < ELFHeader->SectionHeaderCount; I++) {
            elf64_section_header *Header = (vptr)(SectionHeaderBase + I*ELFHeader->SectionHeaderSize);
            
            c08 *Name = StringTable + Header->Name;
            
            if(_strcmp(Name, ".text") == 0) {
               TextSize = (TextSize + PrevTextAlign-1) & ~(PrevTextAlign-1);
               TextSize += Header->Size;
               PrevTextAlign = Header->Align;
            } else if(_strcmp(Name, ".rodata") == 0) {
               RODataSize = (RODataSize + PrevRODataAlign-1) & ~(PrevRODataAlign-1);
               RODataSize += Header->Size;
               PrevRODataAlign = Header->Align;
            } else if(_strcmp(Name, ".data") == 0) {
               DataSize = (DataSize + PrevDataAlign-1) & ~(PrevDataAlign-1);
               DataSize += Header->Size;
               PrevDataAlign = Header->Align;
            } else if(_strcmp(Name, ".bss") == 0) {
               BSSSize = (BSSSize + PrevBSSAlign-1) & ~(PrevBSSAlign-1);
               BSSSize += Header->Size;
               PrevBSSAlign = Header->Align;
            }
         }
         TextSize   = (TextSize   + PrevRODataAlign - 1) & ~(PrevRODataAlign - 1);
         RODataSize = (RODataSize + PrevDataAlign   - 1) & ~(PrevDataAlign   - 1);
         DataSize   = (DataSize   + PrevBSSAlign    - 1) & ~(PrevBSSAlign    - 1);
         
         U64_ToStr(Buffer, BaseAddress, 16);
         SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, u"Kernel .text is at ");
         SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, Buffer);
         SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, u"\r\n");
         
         U64_ToStr(Buffer, BaseAddress + TextSize, 16);
         SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, u"Kernel .rodata is at ");
         SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, Buffer);
         SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, u"\r\n");
         
         U64_ToStr(Buffer, BaseAddress + TextSize + RODataSize, 16);
         SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, u"Kernel .data is at ");
         SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, Buffer);
         SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, u"\r\n");
         
         U64_ToStr(Buffer, BaseAddress + TextSize + RODataSize + DataSize, 16);
         SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, u"Kernel .bss is at ");
         SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, Buffer);
         SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, u"\r\n");
         
         u64 PageCount = (TextSize + RODataSize + DataSize + BSSSize + 0x0FFF) >> 12;
         Status = BootServices->AllocatePages(EFI_AllocateType_Address, EFI_MemoryType_LoaderData, PageCount, (vptr*)&BaseAddress);
         SASSERT(EFI_Status_Success, u"ERROR: Could not allocate pages for kernel image\r\n");
         
         u64 TextOffset = 0;
         u64 RODataOffset = 0;
         u64 DataOffset = 0;
         u64 BSSOffset = 0;
         PrevTextAlign = 1;
         PrevRODataAlign = 1;
         PrevDataAlign = 1;
         PrevBSSAlign = 1;
         for(u32 I = 0; I < ELFHeader->SectionHeaderCount; I++) {
            elf64_section_header *Header = (vptr)(SectionHeaderBase + I*ELFHeader->SectionHeaderSize);
            c08 *Name = StringTable + Header->Name;
            u08 *Section = FileData + Header->Offset;
            b08 ShouldCopy = FALSE;
            
            if(_strcmp(Name, ".text") == 0) {
               TextOffset = (TextOffset + PrevTextAlign-1) & ~(PrevTextAlign-1);
               SectionBases[I] = (vptr)(BaseAddress + TextOffset);
               TextOffset += Header->Size;
               PrevTextAlign = Header->Align;
               ShouldCopy = TRUE;
            } else if(_strcmp(Name, ".rodata") == 0) {
               RODataOffset = (RODataOffset + PrevRODataAlign-1) & ~(PrevRODataAlign-1);
               SectionBases[I] = (vptr)(BaseAddress + TextSize + RODataOffset);
               RODataOffset += Header->Size;
               PrevRODataAlign = Header->Align;
               ShouldCopy = TRUE;
            } else if(_strcmp(Name, ".data") == 0) {
               DataOffset = (DataOffset + PrevDataAlign-1) & ~(PrevDataAlign-1);
               SectionBases[I] = (vptr)(BaseAddress + TextSize + RODataSize + DataOffset);
               DataOffset += Header->Size;
               PrevDataAlign = Header->Align;
               ShouldCopy = TRUE;
            } else if(_strcmp(Name, ".bss") == 0) {
               BSSOffset = (BSSOffset + PrevBSSAlign-1) & ~(PrevBSSAlign-1);
               SectionBases[I] = (vptr)(BaseAddress + TextSize + RODataSize + DataSize + BSSOffset);
               BSSOffset += Header->Size;
               PrevBSSAlign = Header->Align;
            } else {
               SectionBases[I] = 0;
            }
            
            if(ShouldCopy) {
               u08 *Src08 = Section;
               u08 *Dst08 = SectionBases[I];
               for(u64 J = 0; J < Header->Size; J++)
                  *Dst08++ = *Src08++;
            }
         }
         
         for(u32 I = 0; I < ELFHeader->SectionHeaderCount; I++) {
            elf64_section_header *Header = (vptr)(SectionHeaderBase + I*ELFHeader->SectionHeaderSize);
            
            c08 *Name = StringTable + Header->Name;
            u08 *Section = FileData + Header->Offset;
            
            if(_strcmp(Name, ".rela.text") == 0) {
               elf64_section_header *SymbolTable = (vptr)(SectionHeaderBase + Header->Link*ELFHeader->SectionHeaderSize);
               u08 *SymbolsBase = FileData + SymbolTable->Offset;
               
               for(u64 J = 0; J < Header->Size; J += Header->EntrySize) {
                  elf64_relocation_addend *Entry = (vptr)(Section + J);
                  elf64_symbol *Symbol = (vptr)(SymbolsBase + (Entry->Info >> 32)*SymbolTable->EntrySize);
                  u08 *Dest = (vptr)(BaseAddress + Entry->Offset);
                  u64 ReferenceBase = (u64)SectionBases[Symbol->SectionIndex];
                  u64 ReferencerBase = (u64)SectionBases[Header->Info];
                  
                  switch(ELFHeader->Machine) {
                     case ELF_MachineType_AMD64: {
                        switch(Entry->Info & 0xFFFFFFFF) {
                           case ELF_RelocationType_AMD64_64:
                              *(u64*)Dest = ReferenceBase + Symbol->Value + Entry->Addend;
                              break;
                           
                           case ELF_RelocationType_AMD64_PC32:
                              *(u32*)Dest = ReferenceBase + Symbol->Value + Entry->Addend - Entry->Offset - ReferencerBase;
                              break;
                           
                           case ELF_RelocationType_AMD64_PLT32:
                              //TODO//HACK: Well, not sure where the .plt is, so... I guess this works? Maybe?
                              *(u32*)Dest = ReferenceBase + Symbol->Value + Entry->Addend - Entry->Offset - ReferencerBase;
                              break;
                           
                           case ELF_RelocationType_AMD64_32S:
                              *(s32*)Dest = (s32)(s64)(ReferenceBase + Symbol->Value + Entry->Addend);
                              break;
                           
                           case ELF_RelocationType_AMD64_GOTPCRelX:
                              //TODO//HACK: Well, not sure where the .got is, so... I guess this works? Maybe?
                              *(u32*)Dest = ReferenceBase + Symbol->Value + Entry->Addend - Entry->Offset - ReferencerBase;
                              break;
                           
                           case ELF_RelocationType_AMD64_None:
                           case ELF_RelocationType_AMD64_GOT32:
                           case ELF_RelocationType_AMD64_Copy:
                           case ELF_RelocationType_AMD64_Global:
                           case ELF_RelocationType_AMD64_Jump:
                           case ELF_RelocationType_AMD64_Rel:
                           case ELF_RelocationType_AMD64_GOTPCRel:
                           case ELF_RelocationType_AMD64_32:
                           case ELF_RelocationType_AMD64_16:
                           case ELF_RelocationType_AMD64_PC16:
                           case ELF_RelocationType_AMD64_8:
                           case ELF_RelocationType_AMD64_PC8:
                           case ELF_RelocationType_AMD64_DPTMod64:
                           case ELF_RelocationType_AMD64_DTPOff64:
                           case ELF_RelocationType_AMD64_TPOff64:
                           case ELF_RelocationType_AMD64_TLSGD:
                           case ELF_RelocationType_AMD64_TLSLD:
                           case ELF_RelocationType_AMD64_DTPOff32:
                           case ELF_RelocationType_AMD64_GOTTPOff:
                           case ELF_RelocationType_AMD64_TPOff32:
                           default:
                              SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, u"Unsupported AMD64 relocation type!\r\n    Type: ");
                              U64_ToStr(Buffer, Entry->Info & 0xFFFFFFFF, 16);
                              SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, Buffer);
                              SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, u"\r\n    Index: ");
                              U64_ToStr(Buffer, (J & 0xFFFFFFFF) / Header->EntrySize, 10);
                              SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, Buffer);
                              SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, u"\r\n    Section: ");
                              U64_ToStr(Buffer, Header->Offset + J, 16);
                              SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, Buffer);
                              SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, u"\r\n");
                              ASSERT(FALSE, u"");
                              break;
                        }
                     } break;
                     
                     default:
                        ASSERT(FALSE, u"Unsupported ELF machine type!\r\n");
                        break;
                  }
               }
            }
         }
         
         KernelEntryAddress = BaseAddress + ELFHeader->Entry;
         
         Status = BootServices->FreePool(SectionBases);
         SASSERT(EFI_Status_Success, u"ERROR: Could not free section bases\r\n");
      } break;
      
      case ELF_HeaderType_Exec: {
         ASSERT(ELFHeader->ProgramHeaderCount > 0, u"Executable ELF must have at least one program header!\r\n");
         
         elf64_program_header *Header = (vptr)ProgramHeaderBase;
         u64 BaseAddress = Header->VirtualAddress;
         u64 MaxAddress = Header->VirtualAddress + Header->SizeInMemory;
         
         for(u32 I = 1; I < ELFHeader->ProgramHeaderCount; I++) {
            Header = (vptr)(ProgramHeaderBase + I*ELFHeader->ProgramHeaderSize);
            
            switch(Header->Type) {
               case ELF_ProgramHeaderType_Load: {
                  if(BaseAddress > Header->VirtualAddress)
                     BaseAddress = Header->VirtualAddress;
                  if(MaxAddress < Header->VirtualAddress + Header->SizeInMemory)
                     MaxAddress = Header->VirtualAddress + Header->SizeInMemory;
               } break;
               
               case ELF_ProgramHeaderType_GNUStack:
                  break;
               
               default:
                  U64_ToStr(Buffer, Header->Type, 16);
                  SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, u"Unsupported program header type! (");
                  SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, Buffer);
                  SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, u")\r\n");
                  ASSERT(FALSE, u"");
            }
            
         }
         
         U64_ToStr(Buffer, BaseAddress, 16);
         SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, u"NOTE: Base address is at ");
         SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, Buffer);
         SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, u"\r\n");
         
         U64_ToStr(Buffer, MaxAddress, 16);
         SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, u"NOTE: Max address is at ");
         SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, Buffer);
         SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, u"\r\n");
         
         u64 PageCount = (MaxAddress - BaseAddress + 0x0FFF) >> 12;
         Status = BootServices->AllocatePages(EFI_AllocateType_Address, EFI_MemoryType_LoaderData, PageCount, (vptr*)&BaseAddress);
         SASSERT(EFI_Status_Success, u"ERROR: Could not allocate pages for kernel image\r\n");
         
         for(u32 I = 0; I < ELFHeader->ProgramHeaderCount; I++) {
            Header = (vptr)(ProgramHeaderBase + I*ELFHeader->ProgramHeaderSize);
            
            u08 *Dst08 = (u08*)Header->VirtualAddress;
            u08 *Src08 = (u08*)(FileData + Header->Offset);
            for(u32 J = 0; J < Header->SizeInMemory; J++)
               *Dst08++ = *Src08++;
         }
         
         KernelEntryAddress = ELFHeader->Entry;
      } break;
      
      default: ASSERT(FALSE, u"Unsupported ELF type!\r\n");
   }
   
   Status = BootServices->FreePool(FileData);
   SASSERT(EFI_Status_Success, u"ERROR: Could not free kernel file\r\n");
   
   U64_ToStr(Buffer, KernelEntryAddress, 16);
   SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, u"NOTE: Kernel entry point is at ");
   SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, Buffer);
   SystemTable->ConsoleOut->OutputString(SystemTable->ConsoleOut, u"\r\n");
   
   #if 0
    //
    // Renderer and Terminal
    //
    software_renderer Renderer = {0};
    Renderer.Format = GOP->Mode->Info->PixelFormat == EFI_GraphicsPixelFormat_BlueGreenRedReserved8BitPerColor ? PixelFormat_BGRX_8 : PixelFormat_RGBX_8;
    Renderer.Size = (v2u32){GOP->Mode->Info->HorizontalResolution,
                            GOP->Mode->Info->VerticalResolution};
    u64 FramebufferSize = Renderer.Size.X * Renderer.Size.Y * 4;
    u32 *Framebuffer = Context.Allocate(FramebufferSize);
    Renderer.Framebuffer = Framebuffer;
    Renderer.Pitch = 4 * GOP->Mode->Info->PixelsPerScanLine;
    Renderer.BackgroundColor = V4u08(0, 0, 0, 0);
    
    
    terminal Terminal = InitTerminal(200, 4000, FontFile, Renderer.Size);
    Context.Terminal = &Terminal;
    c08 Text[] = "--------Terminal Test--------\nabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ \t 1234567890`~!@#$%^&*()-_=+[{]}\\|;:'\",<.>/?\nNewline: [\n]\nTab: [\t]\n-----------------------------\n\n";
    WriteToTerminal(&Terminal, Text, sizeof(Text)-1);
    //  At size 32: 
    //    --------Terminal Test--------
    //    abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVW
    //    XYZ      1234567890`~!@#$%^&*()-
    //    _=+[{]}\\|;:'\",<.>/?
    //    Newline: [
    //    ]
    //    Tab: [    ]
    //    -----------------------------
    //    
    DrawTerminal(Framebuffer, &Terminal);
    
    
    
    WriteToTerminal(&Terminal, "Image base is at: 0x", 0);
    c08 Buffer[32];
    u32 Index = sizeof(Buffer);
    Buffer[--Index] = 0;
    u64 Value = (u64)LoadedImage->ImageBase;
    do {
        u32 Digit = Value % 16;
        if(Digit < 10) Buffer[--Index] = Digit + '0';
        else           Buffer[--Index] = Digit - 10 + 'A';
        Value /= 16;
    } while(Value > 0);
    WriteToTerminal(&Terminal, Buffer+Index, 0);
    WriteToTerminal(&Terminal, "\n", 1);
    Mem_Set(Framebuffer, 0, FramebufferSize);
    DrawTerminal(Framebuffer, &Terminal);
    GOP->Blt(GOP, (efi_graphics_output_blt_pixel*)Framebuffer, EFI_GraphicsOutputBltOperation_BufferToVideo, 0, 0, 0, 0, Renderer.Size.X, Renderer.Size.Y, 0);
    
    for(u32 I = 0; I < MemoryDescriptorCount; I++) {
        efi_memory_descriptor *Descriptor = (vptr)((u08*)MemoryMap+I*MemoryDescriptorSize);
        
        WriteToTerminal(&Terminal, "Type: ", 0);
        Index = sizeof(Buffer);
        Buffer[--Index] = 0;
        Value = Descriptor->Type;
        do {
            u32 Digit = Value % 16;
            if(Digit < 10) Buffer[--Index] = Digit + '0';
            else           Buffer[--Index] = Digit - 10 + 'A';
            Value /= 16;
        } while(Value > 0);
        WriteToTerminal(&Terminal, Buffer+Index, 0);
        
        WriteToTerminal(&Terminal, " :: Attribute: ", 0);
        Index = sizeof(Buffer);
        Buffer[--Index] = 0;
        Value = Descriptor->Attribute;
        do {
            u32 Digit = Value % 16;
            if(Digit < 10) Buffer[--Index] = Digit + '0';
            else           Buffer[--Index] = Digit - 10 + 'A';
            Value /= 16;
        } while(Value > 0);
        WriteToTerminal(&Terminal, Buffer+Index, 0);
        WriteToTerminal(&Terminal, "\n", 0);
        
        Mem_Set(Framebuffer, 0, FramebufferSize);
        DrawTerminal(Framebuffer, &Terminal);
        GOP->Blt(GOP, (efi_graphics_output_blt_pixel*)Framebuffer, EFI_GraphicsOutputBltOperation_BufferToVideo, 0, 0, 0, 0, Renderer.Size.X, Renderer.Size.Y, 0);
    }
   #endif
   
   rsdp *RSDP = NULL;
   for(u32 I = 0; I < SystemTable->ConfigTableEntryCount; I++) {
      efi_configuration_table ConfigTable = SystemTable->ConfigTable[I];
      if(*(u64*)&ConfigTable.VendorGuid.Data1 == *(u64*)&EFI_GUID_ACPI_TABLE_2_0.Data1 &&
         *(u64*)&ConfigTable.VendorGuid.Data4 == *(u64*)&EFI_GUID_ACPI_TABLE_2_0.Data4) {
         RSDP = ConfigTable.VendorTable;
      }
   }
   
   //TODO: Make these asserts handle instead of crashing
   
   efi_graphics_output_protocol *GOP;
   Status = BootServices->LocateProtocol(&EFI_GUID_GRAPHICS_OUTPUT_PROTOCOL, NULL, (vptr*)&GOP);
   SASSERT(EFI_Status_Success, u"ERROR: Could not load GraphicsOutput Protocol\r\n");
   
   efi_pci_root_bridge_io_protocol *PRBIP;
   Status = BootServices->LocateProtocol(&EFI_GUID_PCI_ROOT_BRIDGE_IO_PROTOCOL, NULL, (vptr*)&PRBIP);
   
   efi_memory_descriptor *MemoryMap;
   u64 MemoryMapKey, MemoryMapSize=0, MemoryMapDescriptorSize=0;
   Status = BootServices->GetMemoryMap(&MemoryMapSize, NULL, NULL, &MemoryMapDescriptorSize, NULL);
   MemoryMapSize += 2*sizeof(efi_memory_descriptor);
   Status = BootServices->AllocatePool(EFI_MemoryType_LoaderData, MemoryMapSize, (vptr*)&MemoryMap);
   SASSERT(EFI_Status_Success, u"ERROR: Could not allocate memory for memory map\r\n");
   Status = BootServices->GetMemoryMap(&MemoryMapSize, MemoryMap, &MemoryMapKey, &MemoryMapDescriptorSize, NULL);
   SASSERT(EFI_Status_Success, u"ERROR: Could not get memory map\r\n");
   Status = BootServices->ExitBootServices(ImageHandle, MemoryMapKey);
   SASSERT(EFI_Status_Success, u"ERROR: Could not exit boot services\r\n");
   
   Status = ((kernel_entry)KernelEntryAddress)(RSDP, GOP, PRBIP, SFSP, MemoryMap, MemoryMapDescriptorSize, MemoryMapSize/MemoryMapDescriptorSize);
   return Status;
   
   
   #undef SASSERT
   #undef ASSERT
   
   _error:
   b08 Wait = TRUE;
   while(Wait)
      asm ("pause");
   
   return Status;
}