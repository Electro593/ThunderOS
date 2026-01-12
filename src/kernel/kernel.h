/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                           **
**  Author: Aria Seiler                                                      **
**                                                                           **
**  This program is in the public domain. There is no implied warranty,      **
**  so use it at your own risk.                                              **
**                                                                           **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef _KERNEL_KERNEL_H
#define _KERNEL_KERNEL_H

#include <shared.h>

typedef struct global_state {
	u64 Flags;
	u16 SerialPort;
} global_state;

typedef enum thunderos_flags { HW_HasSerial = 0x01 } thunderos_flags;

typedef enum cr0_flag {
	CR0_ProtectionEnable   = 0x00000001,
	CR0_MonitorCoprocessor = 0x00000002,
	CR0_Emulation		   = 0x00000004,
	CR0_TaskSwitched	   = 0x00000008,
	CR0_ExtensionType	   = 0x00000010,
	CR0_NumericError	   = 0x00000020,
	CR0_WriteProtect	   = 0x00010000,
	CR0_AlignmentMask	   = 0x00040000,
	CR0_NotWriteThrough	   = 0x20000000,
	CR0_CacheDisable	   = 0x40000000,
	CR0_Paging			   = 0x80000000,
} cr0_flag;

typedef enum cr3_flag {
	CR3_PageWriteThrough = 0x00000008,
	CR3_PageCacheDisable = 0x00000010,
} cr3_flag;

typedef enum cr4_flag {
	CR4_Virtual8086Extensions			= 0x00000001,
	CR4_ProtectedVirtualInterrupts		= 0x00000002,
	CR4_TimeStampDisable				= 0x00000004,
	CR4_DebuggingExtensions				= 0x00000008,
	CR4_PageSizeExtensions				= 0x00000010,
	CR4_PhysicalAddressExtension		= 0x00000020,
	CR4_MachineCheckEnable				= 0x00000040,
	CR4_PageGlobalEnable				= 0x00000080,
	CR4_PerformanceCounter				= 0x00000100,
	CR4_FXSAVEAndFXRSTOR				= 0x00000200,
	CR4_SIMDFloatExceptions				= 0x00000400,
	CR4_UserModeInstructionPreventation = 0x00000800,
	CR4_57BitLinearAddress				= 0x00001000,
	CR4_VMXEnable						= 0x00002000,
	CR4_SMXEnable						= 0x00004000,
	CR4_FSGSBASEEnable					= 0x00010000,
	CR4_PCIDEnable						= 0x00020000,
	CR4_XSAVEAndExtendedStates			= 0x00040000,
	CR4_KeyLocker						= 0x00080000,
	CR4_SMEPEnable						= 0x00100000,
	CR4_SMAPEnable						= 0x00200000,
	CR4_UserProtectionKeys				= 0x00400000,
	CR4_ControlFlowEnforcement			= 0x00800000,
	CR4_SupervisorProtectionKeys		= 0x01000000,
} cr4_flag;

extern u08	PortIn08(u16 Address);
extern u32	PortIn32(u16 Address);
extern void PortOut08(u16 Address, u08 Data);
extern void PortOut32(u16 Address, u32 Data);
extern u64	GetMSR(u32 Base);
extern void SetMSR(u32 Base, u64 Value);
extern void SetGDTR(vptr GDT, u16 Size);
extern void SetIDTR(vptr IDT, u16 Size);
extern u64	GetCR0(void);
extern u64	GetCR3(void);
extern u64	GetCR4(void);
extern void SetCR0(u64);
extern void SetCR3(u64);
extern void DisableInterrupts(void);
extern void EnableInterrupts(void);
extern void InvalidatePage(vptr Address);

extern int link_test(void);

extern global_state Global;

#define Assert(Expression, ...) \
   do { \
      if(!(Expression)) \
         KernelError(__FILE__, __LINE__, #Expression, "" __VA_ARGS__); \
   } while(0);

void KernelError(c08 *File, u32 Line, c08 *Expression, c08 *Message);

#endif
