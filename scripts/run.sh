#!/bin/bash

echo Starting qemu...

qemu-system-x86_64 -cpu qemu64 \
                   -bios ./scripts/ovmf.fd \
                   -drive if=ide,file=./build/disk.qcow2 \
                   -device qemu-xhci,id=xhci \
                   -m 1G \
                   -s -S
