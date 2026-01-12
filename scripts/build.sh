#!/bin/bash

ARCH=$(uname -m | sed s,i[3456789]86,ia32,)

if [ ! -d build ]; then
   mkdir build
fi

if [ -e build/error.txt ]; then
   rm build/error.txt
fi

CFLAGS="-fshort-wchar -fno-strict-aliasing -ffreestanding -fno-stack-protector -fno-stack-check -Wno-builtin-declaration-mismatch -fomit-frame-pointer -fno-asynchronous-unwind-tables -mno-red-zone"
CFLAGS="$CFLAGS -Isrc -Wall -Wextra -Werror -Wno-unused-function -Wno-unused-variable -Wno-unused-parameter -Wno-unused-but-set-variable -Wno-missing-braces -Wno-sign-compare -Wno-pointer-sign -Wno-unused-value"
CFLAGS="$CFLAGS -ggdb3"
# CFLAGS="$CFLAGS -g0"
# CFLAGS="$CFLAGS -Ofast -fno-tree-slp-vectorize"

# gcc $CFLAGS -E src/kernel/entry.c -o build/loader.i
gcc $CFLAGS -fpic -fPIC -c src/kernel/entry.c -o build/loader.o
# gcc $CFLAGS -c src/kernel/kernel.c -o build/kernel.o
unbuffer gcc $CFLAGS --no-pic -masm=intel -c src/kernel/kernel.c -o build/kernel.o 2> /dev/stdout | tee build/error.txt



nasm -g -f elf64 src/kernel/x64.s -o build/asm.o
# nasm -f elf64 src/kernel/x64.s -o build/asm.o

# BYTES=$(cat 'build/error.txt' | wc -c)
# if [ ${BYTES} -ne 0 ]; then
if [ -s build/error.txt ]; then
   exit 1
fi


LFLAGS="-nostdlib -Bsymbolic"
ld $LFLAGS -shared -Tscripts/elf_${ARCH}_efi.lds build/loader.o -o build/loader.so
ld $LFLAGS -r -static -Tscripts/kernel.lds build/kernel.o build/asm.o -o build/kernel.so
# ld $LFLAGS -static -Tscripts/kernel.lds build/kernel.o build/asm.o -o build/kernel.so build/jai_lib.a

if [ $ARCH = "aarch64" ]; then
   EFIARCH="pei-aarch64-little"
else
   EFIARCH="efi-app-${ARCH}"
fi
SECTIONS="-j .text -j .sdata -j .data -j .dynamic -j .dynsym  -j .rel -j .rela -j .reloc"
SECTIONS_DBG="$SECTIONS -j .debug_info -j .debug_abbrev -j .debug_loc -j .debug_aranges -j .debug_line -j .debug_macinfo -j .debug_str -j .debug_line_str"
objcopy $SECTIONS_DBG --target $EFIARCH --subsystem=10 build/loader.so build/loader_dbg
objcopy $SECTIONS     --target $EFIARCH --subsystem=10 build/loader.so build/loader
objcopy -R .note* -R .comment build/kernel.so build/kernel


objdump -l -S -d --source-comment build/loader > build/loader.s
objdump -l -S -d --source-comment build/kernel > build/kernel.s
objdump --all-headers build/loader_dbg > build/loader.dump
objdump --all-headers build/kernel > build/kernel.dump



find build/ -name "*.o"  | xargs rm 2>/dev/null
find build/ -name "*.so" | xargs rm 2>/dev/null



IsMounted=$(mount | grep '/mnt/thunderos')
if [ "$IsMounted" = "" ]; then
   sudo qemu-nbd -c /dev/nbd0 emulator/disk.vhd
   sudo mount -t auto -o rw /dev/nbd0p1 /mnt/thunderos
fi
sudo cp build/loader /mnt/thunderos/EFI/BOOT/BOOTX64.efi
sudo cp build/kernel /mnt/thunderos/kernel
sudo umount /dev/nbd0p1
sudo qemu-nbd -d /dev/nbd0