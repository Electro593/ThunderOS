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

file ./build/loader
# add-symbol-file ./build/loader_dbg 0x3E3AF000
add-symbol-file ./build/kernel -o 0x1000

set architecture i386:x86-64:intel
set disassembly-flavor intel
target remote :1234

b _start

layout split

c