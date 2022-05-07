/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                         **
**  Author: Aria Seiler                                                    **
**                                                                         **
**  This program is in the public domain. There is no implied warranty,    **
**  so use it at your own risk.                                            **
**                                                                         **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifdef INCLUDE_SOURCE

internal u32
Serial_Init(u32 BaudRate, u16 *Port)
{
   u16 Ports[] = {0x3F8,0x2F8,0x3E8,0x2E8,0x5F8,0x4F8,0x5E8,0x4E8};
   for(u32 I = 0; I < 7; I++) {
      PortOut08(Ports[I]+1, 0);
      PortOut08(Ports[I]+3, 0x80);
      PortOut08(Ports[I]+0, 115200/BaudRate);
      PortOut08(Ports[I]+1, 0);
      PortOut08(Ports[I]+3, 0x03);
      PortOut08(Ports[I]+2, 0xC7);
      PortOut08(Ports[I]+4, 0x0B);
      
      PortOut08(Ports[I]+4, 0x1E);
      PortOut08(Ports[I]+0, 0xAE);
      if(PortIn08(Ports[I]+0) == 0xAE) {
         PortOut08(Ports[I]+4, 0x0F);
         *Port = Ports[I];
         return ST_Success;
      }
   }
   return ST_NotSupported;
}

internal void
Serial_ReadChar(u16 Port, c08 *Char)
{
   while(!(PortIn08(Port+5) & 0x01));
   *Char = PortIn08(Port+0);
}

internal void
Serial_WriteChar(u16 Port, c08 Char)
{
   while(!(PortIn08(Port+5) & 0x20));
   PortOut08(Port+0, Char);
}

internal u32
Serial_ReadLine(u16 Port,
                c08 *Buffer,
                u32 BufferSize)
{
   u32 I;
   for(I = 0; I < BufferSize-1; I++) {
      Serial_ReadChar(Port, Buffer+I);
      Serial_WriteChar(Port, Buffer[I]);
      if(I < BufferSize-2 && Buffer[I] == '\r') {
         Serial_WriteChar(Port, '\n');
         Buffer[I+1] = '\n';
         I += 2;
         break;
      }
   }
   Buffer[I] = 0;
   
   return I;
}

internal void
Serial_Write(u16 Port,
             c08 *Buffer)
{
   while(*Buffer)
      Serial_WriteChar(Port, *Buffer++);
}

#endif