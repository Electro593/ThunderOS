gnome-terminal -e "gdb build/ThunderOS.efi"

qemu-system-x86_64 -cpu host -enable-kvm                            \
                   -vga virtio \
                   -bios ./emulator/OVMF.fd               \
                   -drive if=ide,file=./emulator/disk.vhd \
                   -s