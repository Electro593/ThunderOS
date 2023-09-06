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

add-symbol-file ./build/loader.so 0x3E3AD000
add-symbol-file ./build/kernel.so -o 0x1000

set architecture i386:x86-64:intel
set disassembly-flavor intel
target remote :1234

b _start

layout split

c