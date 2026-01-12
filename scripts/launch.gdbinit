define wc_print
  echo "
  set $c = (c16*)$arg0
  while(*$c)
    if(*$c > 0x7f)
      printf "[%x]", *$c
    else
      printf "%c", *$c
    end
    set $c++
  end
  echo "\n
end

pwd

info files
file
# add-symbol-file kernel 0x1000

set architecture i386:x86-64:intel
set disassembly-flavor intel

break Kernel_Entry
