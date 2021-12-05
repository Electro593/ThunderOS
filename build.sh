if [ ! -d build ]
then
    mkdir ./build
fi

# # CompilerOptions="$CompilerOptions -ffreestanding -Werror -Wall -Wextra -g -Og -fpie -pie -nostdlib"
# # CompilerOptions="$CompilerOptions -Wno-error=unused-function -Wno-error=unused-but-set-variable"
# # echo $CompilerOptions

# # gcc ./src/kernel/entry.c -Isrc -o ./build/ThunderOS.efi -e EFI_Entry $CompilerOptions

#     # -ffreestanding          \
#     # -Isrc                   \

# # nasm ./src/kernel/x64/start.asm \
# #      -o ./build/start.o

# gcc -Isrc                      \
#     -fpic                      \
#     -ffreestanding             \
#     -fno-stack-protector       \
#     -fno-stack-check           \
#     -mno-red-zone              \
#     -fshort-wchar              \
#     -maccumulate-outgoing-args \
#     -c                         \
#     ./src/kernel/entry.c       \
#     -o ./build/ThunderOS.o     \
#     -ggdb3

    
# #    -znocombreloc       \
# #    -nostdlib           \

# ld -shared                     \
#    -Bsymbolic                  \
#    -Lgnuefi/                   \
#    -Tgnuefi/elf_x86_64_efi.lds \
#    ./gnuefi/crt0-efi-x86_64.o  \
#    ./build/ThunderOS.o         \
#    -o ./build/ThunderOS.so     \
#    -lgnuefi                    \
#    -lefi                       \

# objcopy -j .text                \
#         -j .sdata               \
#         -j .data                \
#         -j .dynamic             \
#         -j .dynsym              \
#         -j .rel                 \
#         -j .rela                \
#         -j .reloc               \
#         --target=efi-app-x86_64 \
#         --subsystem=10          \
#         ./build/ThunderOS.so    \
#         ./build/ThunderOS.efi

# objcopy -j .text                \
#         -j .sdata               \
#         -j .data                \
#         -j .dynamic             \
#         -j .dynsym              \
#         -j .rel                 \
#         -j .rela                \
#         -j .reloc               \
#         -j .debug_info          \
#         -j .debug_abbrev        \
#         -j .debug_loc           \
#         -j .debug_aranges       \
#         -j .debug_ranges        \
#         -j .debug_line          \
#         -j .debug_macinfo       \
#         -j .debug_str           \
#         --target=efi-app-x86_64 \
#         --subsystem=10          \
#         ./build/ThunderOS.so    \
#         ./build/ThunderOS_Debug.efi

# # objdump -S --disassemble ./build/ThunderOS.efi > ./build/Listing.asm
# objdump -S --disassemble ./build/ThunderOS_Debug.efi > ./build/Listing.asm
# objdump --all-headers ./build/ThunderOS_Debug.efi > ./build/Dump

# # x86_64-w64-mingw32-gcc ./src/kernel/entry.c -Isrc -o ./build/ThunderOS.efi -ffreestanding -nostdlib -Wl,-dll -shared -Wl,--subsystem,10 -e EFI_Entry
# # x86_64-w64-mingw32-gcc ./src/kernel/entry.c -Isrc -o ./build/ThunderOS_Debug.efi -ffreestanding -nostdlib -Wl,-dll -shared -Wl,--subsystem,10 -e EFI_Entry

# # x86_64-w64-mingw32-gcc ./src/kernel/entry.c -Isrc -o ./build/ThunderOS.efi       -ffreestanding -nostdlib -fno-stack-protector -fpic -fshort-wchar -mno-red-zone -Wl,-dll -shared -Wl,--subsystem,10 -Bsymbolic       -e EFI_Entry
# # x86_64-w64-mingw32-gcc ./src/kernel/entry.c -Isrc -o ./build/ThunderOS_Debug.efi -ffreestanding -nostdlib -fno-stack-protector -fpic -fshort-wchar -mno-red-zone -Wl,-dll -shared -Wl,--subsystem,10 -Bsymbolic -ggdb -e EFI_Entry




























rm uefi/*.o uefi/libuefi.a 2>/dev/null






make
DEBUG=1 make

find src/ -name "*.o" | xargs rm
objdump -S --disassemble ./build/ThunderOS.efi > ./build/dump.asm


sudo qemu-nbd -c /dev/nbd0 ./emulator/disk.vhd
sudo mount -t auto -o rw /dev/nbd0p1 /mnt
sudo cp ./build/ThunderOS.efi /mnt/EFI/BOOT/BOOTX64.efi
# sudo cp ./build/ThunderOS_Debug.efi /mnt/EFI/BOOT/BOOTX64dbg.efi
sudo umount /dev/nbd0p1
sudo qemu-nbd -d /dev/nbd0