ROOT := ../../

SCRIPTS := $(ROOT)scripts/
OUT := $(ROOT)build/
TMP := $(ROOT)build/src/loader/

LDS := $(SCRIPTS)elf_$(ARCH)_efi.lds

CFLAGS := -fshort-wchar -fno-strict-aliasing -ffreestanding -fno-stack-check
CFLAGS += -fno-stack-protector -fomit-frame-pointer -fno-asynchronous-unwind-tables
CFLAGS += -mno-red-zone -masm=intel -I$(ROOT)src -I$(ROOT)src/loader -ggdb3
CFLAGS += $(CWARNS)

LDFLAGS := -nostdlib -Bsymbolic -shared -T$(LDS)

SOURCES := $(wildcard *.c) $(wildcard */*.c)
OBJECTS := $(SOURCES:%.c=$(TMP)%.o)

DIRS := build/ $(patsubst %,build/src/loader/%,$(dir $(SOURCES)))

.PHONY: loader

loader: $(DIRS) $(OUT)loader $(OUT)loader.dump.asm $(OUT)loader.dump.dat

$(OUT)loader: $(OUT)loader.so
	objcopy -j .text -j .sdata -j .data -j .dynamic -j .dynsym  -j .rel -j .rela -j .reloc \
		--target $(EFIARCH) --subsystem=10 $< $@

$(OUT)loader_dbg: $(OUT)loader.so
	objcopy -j .text -j .sdata -j .data -j .dynamic -j .dynsym  -j .rel -j .rela -j .reloc \
		-j .debug_info -j .debug_abbrev -j .debug_loc -j .debug_aranges \
		-j .debug_line -j .debug_macinfo -j .debug_str -j .debug_line_str \
		--target $(EFIARCH) --subsystem=10 $< $@

$(OUT)loader.so: $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) -o $@

$(OUT)loader.dump.asm: $(OUT)loader
	objdump -l -S -d --source-comment -M intel $< > $@

$(OUT)loader.dump.dat: $(OUT)loader_dbg
	objdump -x -s $< > $@

$(TMP)%.o: %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

%/:
	cd $(ROOT) && mkdir -p $@
