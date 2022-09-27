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

info files
file
add-symbol-file ./build/kernel 0x1000

set architecture i386:x86-64:intel
set disassembly-flavor intel
target remote :1234

break Kernel_Entry
break KernelError

b Exception_PageFault
b InterruptSwitch.End

layout split

c