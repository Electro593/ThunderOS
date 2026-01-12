/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                           **
**  Author: Aria Seiler                                                      **
**                                                                           **
**  This program is in the public domain. There is no implied warranty,      **
**  so use it at your own risk.                                              **
**                                                                           **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef _DRIVERS_SERIAL_H
#define _DRIVERS_SERIAL_H

#include <shared.h>

u32	 Serial_Init(u32 BaudRate, u16 *Port);
void Serial_ReadChar(u16 Port, c08 *Char);
void Serial_WriteChar(u16 Port, c08 Char);
u32	 Serial_ReadLine(u16 Port, c08 *Buffer, u32 BufferSize);
void Serial_Write(u16 Port, c08 *Buffer);

#endif
