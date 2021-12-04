@REM call "C:/Program Files/qemu/qemu-system-x86_64" -bios D:\Programming\C\ThunderOS\virtualbox\OVMF-pure-efi.fd
call "C:/Program Files/qemu/qemu-system-x86_64" -S -cpu qemu64 -bios D:\Programming\C\ThunderOS\virtualbox\OVMF-pure-efi.fd
@REM call "C:/Program Files/qemu/qemu-system-x86_64" -cpu qemu64 -bios D:\Programming\C\ThunderOS\virtualbox\OVMF-pure-efi.fd -drive file=build/OS.efi
@REM call "C:/Program Files/qemu/qemu-system-x86_64" -drive file=x86_64.img,format=raw