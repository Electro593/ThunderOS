# konsole -e "gdb build/loader_dbg" &
konsole -e "gdb" &

qemu-system-x86_64 -cpu qemu64 \
                   -bios ./emulator/OVMF.fd               \
                   -drive if=ide,file=./emulator/disk.vhd \
                   -device qemu-xhci,id=xhci \
                   -m 1G \
                   -s -S
