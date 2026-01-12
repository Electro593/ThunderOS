export AS := nasm
export CC := gcc
export LD := ld

export ARCH := $(shell uname -m | sed s,i[3456789]86,ia32,)
ifeq ($(ARCH), aarch64)
   export EFIARCH := pei-aarch64-little
else
   export EFIARCH := efi-app-$(ARCH)
endif

ASFLAGS := -g -f elf64

LDFLAGS := -nostdlib -Bsymbolic

CWARNS := -Wall -Wextra -Werror
CWARNS += -Wno-unused-function -Wno-unused-variable -Wno-unused-parameter
CWARNS += -Wno-unused-but-set-variable -Wno-missing-braces -Wno-sign-compare
CWARNS += -Wno-pointer-sign -Wno-unused-value -Wno-builtin-declaration-mismatch
CWARNS += -Wno-unterminated-string-initialization
export CWARNS

CFLAGS := -fshort-wchar -fno-strict-aliasing -ffreestanding -fno-stack-check
CFLAGS += -fno-stack-protector -fomit-frame-pointer -fno-asynchronous-unwind-tables
CFLAGS += -mno-red-zone -masm=intel -Isrc -ggdb3
CFLAGS += $(CWARNS)

c_sources := $(wildcard src/*/*.c)
s_sources := $(wildcard src/*/*.s)

loader_c_sources := src/loader/entry.c
kernel_c_sources := $(filter-out $(loader_c_sources),$(c_sources))
kernel_s_sources := $(s_sources)

loader_objects := $(loader_c_sources:%.c=build/%.o)
kernel_objects := $(kernel_c_sources:%.c=build/%.o) $(kernel_s_sources:%.s=build/%.o)

dirs := build/ $(patsubst %,build/%,$(dir $(c_sources) $(s_sources)))

.PHONY: mount all loader kernel dirs clean

test:
	cd src/loader && $(MAKE)

mount: all
	./scripts/mount.sh

all: loader kernel

loader: dirs build/loader build/loader_dbg build/loader.dump.asm build/loader.dump.dat

kernel: dirs build/kernel build/kernel.dump.asm build/kernel.dump.dat

dirs: $(dirs)

build/loader: build/loader.so
	objcopy -j .text -j .sdata -j .data -j .dynamic -j .dynsym  -j .rel -j .rela -j .reloc \
		--target $(EFIARCH) --subsystem=10 $< $@

build/loader_dbg: build/loader.so
	objcopy -j .text -j .sdata -j .data -j .dynamic -j .dynsym  -j .rel -j .rela -j .reloc \
		-j .debug_info -j .debug_abbrev -j .debug_loc -j .debug_aranges \
		-j .debug_line -j .debug_macinfo -j .debug_str -j .debug_line_str \
		--target $(EFIARCH) --subsystem=10 $< $@

build/kernel: build/kernel.so
	objcopy -R .note* -R .comment -R .group $< $@

build/loader.so: $(loader_objects)
	$(LD) $(LDFLAGS) -shared -Tscripts/elf_$(ARCH)_efi.lds $(loader_objects) -o build/loader.so

build/kernel.so: $(kernel_objects)
# 	$(LD) $(LDFLAGS) -Tscripts/kernel.lds $(kernel_objects) -o build/kernel.so
	$(LD) -nostdlib $(kernel_objects) -o build/kernel.so

build/%.dump.asm: build/%
	objdump -l -S -d --source-comment -M intel $< > $@

build/loader.dump.dat: build/loader_dbg
	objdump -x -s $< > $@

build/kernel.dump.dat: build/kernel
	readelf -a $< > $@

build/%.o: %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

build/%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

%/:
	mkdir -p $@

clean:
	rm -rf build/
