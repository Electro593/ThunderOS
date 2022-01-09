COMPILER="GCC"
ARCH=$(uname -m | sed s,i[3456789]86,ia32,)
SRCS="src/kernel/entry.c"
TARGET="build/ThunderOS.efi"
TARGET_DBG="build/ThunderOS_Debug.efi"
CFLAGS="-fshort-wchar -fno-strict-aliasing -ffreestanding -fno-stack-protector -fno-stack-check -Iuefi -Isrc"
# CFLAGS="$CFLAGS -Ofast -fno-tree-slp-vectorize"
CFLAGS="$CFLAGS -ggdb3"
LFLAGS="-nostdlib -shared -Bsymbolic"
SECTIONS="-j .text -j .sdata -j .data -j .dynamic -j .dynsym  -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc"
SECTIONS_DBG="$SECTIONS -j .debug_info -j .debug_abbrev -j .debug_loc -j .debug_aranges -j .debug_line -j .debug_macinfo -j .debug_str"

if [ ! -d build ]; then
    mkdir build
fi

if [ $ARCH = "x86_64" ]; then
    CFLAGS="$CFLAGS -DHAVE_USE_MS_ABI -mno-red-zone"
fi

if [ $COMPILER = "GCC" ]; then
    if [ $ARCH = "x86_64" ]; then
        CFLAGS="$CFLAGS -maccumulate-outgoing-args"
    fi
    CFLAGS="$CFLAGS -Wno-builtin-declaration-mismatch -fpic -fPIC"
    LIBS="-o ${TARGET}.so -Telf_${ARCH}_efi.lds"
    LIBS_DBG="-o ${TARGET_DBG}.so -Telf_${ARCH}_efi.lds"
    if [ $ARCH = "aarch64" ]; then
        EFIARCH="pei-aarch64-little"
    else
        EFIARCH="efi-app-${ARCH}"
    fi
else
    CFLAGS="$CFLAGS --target=${ARCH}-pc-win32-coff -Wno-builtin-requires-header -Wno-incompatible-library-redeclaration -Wno-long-long"
    LFLAGS="$LFLAGS -subsystem:efi_application -nodefaultlib -dll -entry:EFI_Entry"
    LIBS_DBG="-out:${TARGET_DBG}"
fi

for SRC in $SRCS; do
    # gcc $CFLAGS -c -Wa,-adhln $SRC -o /dev/null > build/listing.s
    gcc $CFLAGS -E $SRC -o build/preprocessed.i
    gcc $CFLAGS -c $SRC -o ${SRC}.o
    # gcc $CFLAGS_DBG -c $SRC -o ${SRC}.o
    OBJS="$OBJS ${SRC}.o"
done

ld  $LFLAGS $OBJS $LIBS
ld  $LFLAGS $OBJS $LIBS_DBG
if [ $COMPILER = "GCC" ]; then
    objcopy $SECTIONS --target $EFIARCH --subsystem=10 ${TARGET}.so $TARGET
    objcopy $SECTIONS_DBG --target $EFIARCH --subsystem=10 ${TARGET_DBG}.so $TARGET_DBG
fi

# objdump -l -S -d --source-comment build/ThunderOS.efi > build/listing.asm
objdump -l -S -d --source-comment build/ThunderOS_Debug.efi > build/listing.asm
objdump --all-headers build/ThunderOS_Debug.efi > build/dump

find src/ build/ -name "*.o"  | xargs rm 2>/dev/null
# find src/ build/ -name "*.so" | xargs rm 2>/dev/null


sudo qemu-nbd -c /dev/nbd0 emulator/disk.vhd
sudo mount -t auto -o rw /dev/nbd0p1 /mnt
sudo cp build/ThunderOS.efi /mnt/EFI/BOOT/BOOTX64.efi
sudo umount /dev/nbd0p1
sudo qemu-nbd -d /dev/nbd0