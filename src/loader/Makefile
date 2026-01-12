ROOT := ../../
NAME := loader

TMP := $(OUT)$(SRC)$(NAME)/

RSCRIPTS := $(ROOT)$(SCRIPTS)
RSRC := $(ROOT)$(SRC)
ROUT := $(ROOT)$(OUT)
RTMP := $(ROOT)$(TMP)

LDS := $(RSCRIPTS)elf_$(ARCH)_efi.lds

CFLAGS := -fshort-wchar -fno-strict-aliasing -ffreestanding -fno-stack-check
CFLAGS += -fno-stack-protector -fomit-frame-pointer -fno-asynchronous-unwind-tables
CFLAGS += -mno-red-zone -masm=intel -I$(RSRC) -I$(RSRC)$(NAME) -ggdb3
CFLAGS += $(CWARNS)

LDFLAGS := -nostdlib -Bsymbolic -shared -T$(LDS)

SOURCES := $(wildcard *.c) $(wildcard */*.c)
OBJECTS := $(SOURCES:%.c=$(RTMP)%.o)

DIRS := $(ROUT) $(patsubst %,$(RTMP)%,$(dir $(SOURCES)))

.PHONY: loader

loader: $(DIRS) $(ROUT)loader $(ROUT)loader.dump.asm $(ROUT)loader.dump.dat

$(ROUT)loader: $(ROUT)loader.so
	objcopy -j .text -j .sdata -j .data -j .dynamic -j .dynsym  -j .rel -j .rela -j .reloc \
		--target $(EFIARCH) --subsystem=10 $< $@

$(ROUT)loader_dbg: $(ROUT)loader.so
	objcopy -j .text -j .sdata -j .data -j .dynamic -j .dynsym  -j .rel -j .rela -j .reloc \
		-j .debug_info -j .debug_abbrev -j .debug_loc -j .debug_aranges \
		-j .debug_line -j .debug_macinfo -j .debug_str -j .debug_line_str \
		--target $(EFIARCH) --subsystem=10 $< $@

$(ROUT)loader.so: $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) -o $@

$(ROUT)loader.dump.asm: $(ROUT)loader
	objdump -l -S -d --source-comment -M intel $< > $@

$(ROUT)loader.dump.dat: $(ROUT)loader_dbg
	objdump -x -s $< > $@

$(RTMP)%.o: %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

$(ROOT)%/:
	cd $(ROOT) && mkdir -p $*
