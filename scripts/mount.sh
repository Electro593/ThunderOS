if [ ! -e "/dev/nbd0" ]; then
   sudo modprobe nbd max_part=8
fi

IsMounted=$(mount | grep '/mnt/thunderos')
if [ "$IsMounted" = "" ]; then
   if [ ! -e "build/disk.qcow2" ]; then
      qemu-img create -f qcow2 build/disk.qcow2 4G
   fi

   if [ ! -e "/mnt/thunderos" ]; then
      sudo mkdir -p /mnt/thunderos
   fi

   sudo qemu-nbd -c /dev/nbd0 build/disk.qcow2
   sudo mkfs.fat -F 32 /dev/nbd0
   sudo mount -t auto -o rw /dev/nbd0p1 /mnt/thunderos
fi

if [ ! -e "/mnt/thunderos/EFI/BOOT" ]; then
   sudo mkdir -p /mnt/thunderos/EFI/BOOT
fi

sudo cp build/loader /mnt/thunderos/EFI/BOOT/BOOTX64.efi
sudo cp build/kernel /mnt/thunderos/kernel
sudo umount /dev/nbd0p1
sudo qemu-nbd -d /dev/nbd0
