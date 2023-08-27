if [ -e "build/loader.so" ]; then
   ARCH=$(uname -m | sed s,i[3456789]86,ia32,)



   if [ $ARCH = "aarch64" ]; then
      EFIARCH="pei-aarch64-little"
   else
      EFIARCH="efi-app-${ARCH}"
   fi
   objcopy -R .note* -R .comment -R .gnu* -R .hash -R .rela* -R .plt -R .eh_frame -R .dynsm -R .dynstr --target $EFIARCH --subsystem=10 build/loader.so build/loader_dbg
   objcopy -R .note* -R .comment -R .gnu* -R .hash -R .rela* -R .plt -R .eh_frame -R .dynsm -R .dynstr -R .debug* --target $EFIARCH --subsystem=10 build/loader.so build/loader
   objcopy build/kernel.so build/kernel;



   objdump -l -S -d -Mintel --source-comment build/loader.so > build/loader.s
   objdump -l -S -d -Mintel --source-comment build/kernel.so > build/kernel.s
   objdump -x -t -r build/loader > build/loader.dump
   objdump -x -t -T -r -R build/kernel > build/kernel.dump



   find build/ -name "*.o"  | xargs rm 2>/dev/null
   find build/ -name "*.so" | xargs rm 2>/dev/null



   IsMounted=$(mount | grep '/mnt')
   if [ "$IsMounted" = "" ]; then
      sudo qemu-nbd -c /dev/nbd0 emulator/disk.vhd
      sudo mount -t auto -o rw /dev/nbd0p1 /mnt
   fi
   sudo cp build/loader /mnt/EFI/BOOT/BOOTX64.efi
   sudo cp build/kernel /mnt/kernel
   sudo umount /dev/nbd0p1
   sudo qemu-nbd -d /dev/nbd0
fi