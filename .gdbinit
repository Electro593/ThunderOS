set pagination off

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

set architecture i386:x86-64:intel
set disassembly-flavor intel
set osabi none

target remote :1234

add-symbol-file ./build/kernel 0x2000
break Kernel_Entry

# add-symbol-file ./build/loader_dbg 0x3E444000
break EFI_Entry

layout split

c
