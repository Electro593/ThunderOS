/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                         **
**  Author: Aria Seiler                                                    **
**                                                                         **
**  This program is in the public domain. There is no implied warranty,    **
**  so use it at your own risk.                                            **
**                                                                         **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

internal void
InitSerial(void)
{
   PortOut08(Port_Serial_COM1+1, 0);
   
   u08 LineControl = 0b00000011;
   PortOut08(Port_Serial_COM1+3, 0x80|LineControl);
   PortOut08(Port_Serial_COM1+0, 3);
   PortOut08(Port_Serial_COM1+1, 0);
   PortOut08(Port_Serial_COM1+3, LineControl);
   
   PortOut08(Port_Serial_COM1+1, Serial_Interrupt_All);
}