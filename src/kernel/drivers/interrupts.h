/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                           **
**  Author: Aria Seiler                                                      **
**                                                                           **
**  This program is in the public domain. There is no implied warranty,      **
**  so use it at your own risk.                                              **
**                                                                           **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef _DRIVERS_INTERRUPTS_H
#define _DRIVERS_INTERRUPTS_H

#include <shared.h>

void Exception_DivideByZero(void);
void Exception_Debug(void);
void Exception_NonMaskableInterrupt(void);
void Exception_Breakpoint(void);
void Exception_Overflow(void);
void Exception_BoundsCheck(void);
void Exception_InvalidOpcode(void);
void Exception_DeviceNotAvailable(void);
void Exception_DoubleFault(u32 ErrorCode);
void Exception_InvalidTSS(u32 ErrorCode);
void Exception_SegmentNotPresent(u32 ErrorCode);
void Exception_StackSegmentFault(u32 ErrorCode);
void Exception_GeneralProtection(u32 ErrorCode);
void Exception_PageFault(u32 ErrorCode);
void Exception_FloatingPoint(void);
void Exception_AlignmentCheck(u32 ErrorCode);
void Exception_MachineCheck(void);
void Exception_SIMDFloatingPoint(void);
void Exception_Virtualization(void);
void Exception_ControlProtection(u32 ErrorCode);
void Exception_HypervisorInjection(void);
void Exception_VMMCommunication(u32 ErrorCode);
void Exception_Security(u32 ErrorCode);
void Interrupt_Spurious(void);

#endif
