#!/bin/bash

# konsole -e "gdb build/loader_dbg" &
# konsole -e "gdb" &

echo Starting qemu...

qemu-system-x86_64 -cpu qemu64 \
                   -bios ./emulator/OVMF.fd \
                   -drive if=ide,file=./build/disk.qcow2 \
                   -device qemu-xhci,id=xhci \
                   -m 1G \
                   -s -S
