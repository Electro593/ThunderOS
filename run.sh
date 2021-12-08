gnome-terminal -e "gdb build/ThunderOS.efi"

qemu-system-x86_64 -cpu qemu64                            \
                   -bios ./emulator/OVMF.fd               \
                   -drive if=ide,file=./emulator/disk.vhd \
                   -s