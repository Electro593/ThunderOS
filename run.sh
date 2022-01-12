# konsole -e "gdb" &
# konsole -e "gdb build/ThunderOS.efi" &
../essence/gf/gf2 build/ThunderOS_Debug.efi &
# gnome-terminal -e "gdb build/ThunderOS.efi"

qemu-system-x86_64 -cpu qemu64                            \
                   -bios ./emulator/OVMF.fd               \
                   -drive if=ide,file=./emulator/disk.vhd \
                   -s -S