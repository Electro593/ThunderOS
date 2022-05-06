# konsole -e "gdb" &
konsole -e "gdb build/ThunderOS_Debug.efi" &
# gnome-terminal -e "gdb build/ThunderOS.efi"

# ../gf/gf2 build/ThunderOS_Debug.efi &

qemu-system-x86_64 -cpu qemu64 \
                   -bios ./emulator/OVMF.fd               \
                   -drive if=ide,file=./emulator/disk.vhd \
                   -device qemu-xhci,id=xhci \
                   -serial file:build/console.out \
                   -m 1G \
                   -s -S
