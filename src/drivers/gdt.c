/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                         **
**  Author: Aria Seiler                                                    **
**                                                                         **
**  This program is in the public domain. There is no implied warranty,    **
**  so use it at your own risk.                                            **
**                                                                         **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

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

internal void
GDT_Init(void)
{
   
}
