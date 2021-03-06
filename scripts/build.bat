@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
if not exist build mkdir build
pushd build

REM Assembler
@REM nasm -Xvc -f coff -Werror=all -o uefi.o ..\src\amd64\bootloader\uefi_entry.asm

set CompilerSwitches=%CompilerSwitches% /nologo /std:c17 /fp:fast /EHa- /FAs /FC /GF /GR- /GS- /Gs0x100000 /J /WX /Wall /X
set CompilerSwitches=%CompilerSwitches% /wd4820 /wd4100 /wd4061
set LinkerSwitches=%LinkerSwitches% /wx /incremental:no /opt:ref /opt:icf /nodefaultlib /stack:0x100000,0x100000 /subsystem:efi_application /machine:x64

REM Compiler
cl %CompilerSwitches% /I ..\src\ ..\src\kernel\entry.c /link %LinkerSwitches% /entry:EFI_Entry /out:OS.efi

copy OS.efi M:\EFI\BOOT\BOOTX64.efi

popd
exit /b 0

REM D:\Programming\C\ThunderOS\virtualbox\disk.vhd
REM C:\Users\seilerar\programming\c\ThunderOS\virtualbox\disk.vhd