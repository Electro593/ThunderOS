AS := nasm
CC := gcc
LD := ld

ASFLAGS := -g -f elf64

LDFLAGS := -nostdlib -Bsymbolic

CWARNS := -Wall -Wextra -Werror
CWARNS += -Wno-unused-function -Wno-unused-variable -Wno-unused-parameter
CWARNS += -Wno-unused-but-set-variable -Wno-missing-braces -Wno-sign-compare
CWARNS += -Wno-pointer-sign -Wno-unused-value -Wno-builtin-declaration-mismatch

CFLAGS := -fshort-wchar -fno-strict-aliasing -ffreestanding -fno-stack-check
CFLAGS += -fno-stack-protector -fomit-frame-pointer -fno-asynchronous-unwind-tables
CFLAGS += -mno-red-zone -masm=intel -Isrc -ggdb3
CFLAGS += $(CWARNS)

ARCH := $(shell uname -m | sed s,i[3456789]86,ia32,)
ifeq ($(ARCH), aarch64)
   EFIARCH := pei-aarch64-little
else
   EFIARCH := efi-app-$(ARCH)
endif

c_sources := $(wildcard src/*/*.c)
s_sources := $(wildcard src/*/*.s)

loader_c_sources := src/loader/entry.c
kernel_c_sources := $(filter-out $(loader_sources),$(c_sources))
kernel_s_sources := $(s_sources)

loader_objects := $(loader_c_sources:%.c=build/%.o)
kernel_objects := $(kernel_c_sources:%.c=build/%.o) $(kernel_s_sources:%.s=build/%.o)

dirs := build/ $(patsubst %,build/%,$(dir $(sources)))

.PHONY: mount all loader kernel clean

mount: all
	./scripts/mount.sh

all: loader kernel

loader: $(dirs) build/loader build/loader.dump.asm build/loader.dump.dat

kernel: $(dirs) build/kernel build/kernel.dump.asm build/kernel.dump.dat

build/loader: build/loader.so
	objcopy -j .text -j .sdata -j .data -j .dynamic -j .dynsym  -j .rel \
		-j .rela -j .reloc --target $(EFIARCH) --subsystem=10 $< $@

build/kernel: build/kernel.so
	objcopy -R .note* -R .comment $< $@

build/loader.so: $(loader_objects)
	$(LD) $(LDFLAGS) -shared -Tscripts/elf_$(ARCH)_efi.lds $(loader_objects) -o build/loader.so

build/kernel.so: $(kernel_objects)
	$(LD) $(LDFLAGS) -r -static -Tscripts/kernel.lds $(kernel_objects) -o build/kernel.so

build/%.dump.asm: build/%
	objdump -l -S -d --source-comment -M intel $< > $@

build/%.dump.dat: build/%
	objdump --all-headers $< > $@

build/%.o: %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

build/%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

%/:
	mkdir -p $@

clean:
	rm -rf build/
