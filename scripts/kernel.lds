ENTRY(Kernel_Entry)
SECTIONS
{
  . = 4096;

  .text : ALIGN(4096)
  {
    *(.text)
  }

  .rodata : ALIGN(4096)
  {
    *(.rodata)
  }

  .data : ALIGN(4096)
  {
    *(.data)
    # *(.data.rel)
    # *(.data.rel.local)
  }

  .bss : ALIGN(4096)
  {
    *(COMMON)
    *(.bss)
  }
}