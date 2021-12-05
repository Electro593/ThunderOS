# qemu-system-x86_64 -bios ./virtualbox/OVMF-pure-efi.fd
# qemu-system-x86_64 -m 64M -bios /usr/share/ovmf/OVMF.fd -cdrom ./emulator/disk.vhd
# qemu-system-x86_64 -drive if=pflash,format=raw,unit=0,readonly,file=./emulator/OVMF_CODE.fd \
#                    -drive if=pflash,format=raw,unit=1,file=./emulator/OVMF_VARS.fd
# qemu-system-x86_64 -L ./emulator

# qemu-system-x86_64 -cpu qemu64 \
#                    -m 1024M \
#                    -drive if=pflash,format=raw,unit=0,file=./emulator/OVMF_CODE.fd,readonly=on \
#                    -drive if=pflash,format=raw,unit=1,file=./emulator/OVMF_VARS.fd \
#                    -drive if=ide,file=./emulator/disk.vhd \
#                    -s \
#                    -net none

qemu-system-x86_64 -cpu qemu64                            \
                   -bios ./emulator/OVMF.fd               \
                   -drive if=ide,file=./emulator/disk.vhd \
                   -s                                     \