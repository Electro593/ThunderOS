if [ -e "build/loader.so" ]; then
   ARCH=$(uname -m | sed s,i[3456789]86,ia32,)



   if [ $ARCH = "aarch64" ]; then
      EFIARCH="pei-aarch64-little"
   else
      EFIARCH="efi-app-${ARCH}"
   fi
   SECTIONS="-j .text -j .sdata -j .data -j .dynamic -j .dynsym  -j .rel -j .rela -j .reloc"
   SECTIONS_DBG="$SECTIONS -j .debug_info -j .debug_abbrev -j .debug_loc -j .debug_aranges -j .debug_line -j .debug_macinfo -j .debug_str -j .debug_line_str"
   objcopy $SECTIONS_DBG --target $EFIARCH --subsystem=10 build/loader.so build/loader_dbg
   objcopy $SECTIONS     --target $EFIARCH --subsystem=10 build/loader.so build/loader
   objcopy -R .note* -R .comment build/kernel.so build/kernel



   objdump -l -S -d -Mintel --source-comment build/loader.so > build/loader.s
   objdump -l -S -d -Mintel --source-comment build/kernel > build/kernel.s
   objdump --all-headers build/loader > build/loader.dump
   objdump --all-headers build/kernel > build/kernel.dump



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