ROOT := ../../
NAME := kernel

TMP := $(OUT)$(SRC)$(NAME)/

RSCRIPTS := $(ROOT)$(SCRIPTS)
RSRC := $(ROOT)$(SRC)
ROUT := $(ROOT)$(OUT)
RTMP := $(ROOT)$(TMP)

LDS := $(RSCRIPTS)kernel.lds

ASFLAGS := -g -f elf64

LDFLAGS := -nostdlib -Bsymbolic -T$(LDS) -static -r

CFLAGS := -fshort-wchar -fno-strict-aliasing -ffreestanding -fno-stack-check
CFLAGS += -fno-stack-protector -fomit-frame-pointer -fno-asynchronous-unwind-tables
CFLAGS += -mno-red-zone -masm=intel -I$(RSRC) -I$(RSRC)$(NAME) -ggdb3
CFLAGS += $(CWARNS)

CSOURCES := $(wildcard *.c */*.c)
SSOURCES := $(wildcard *.s */*.s)

OBJECTS := $(CSOURCES:%.c=$(RTMP)%.o) $(SSOURCES:%.s=$(RTMP)%.o)

DIRS := $(ROUT) $(patsubst %,$(RTMP)%,$(dir $(CSOURCES) $(SSOURCES)))

.PHONY: kernel

kernel: $(DIRS) $(ROUT)kernel $(ROUT)kernel.dump.asm $(ROUT)kernel.dump.dat

$(ROUT)kernel: $(ROUT)kernel.so
	objcopy -R .note* -R .comment -R .group $< $@

$(ROUT)kernel.so: $(OBJECTS)
# 	$(LD) $(LDFLAGS) $(OBJECTS) -o $@
	$(LD) -nostdlib $(OBJECTS) -o $@

$(ROUT)kernel.dump.asm: $(ROUT)kernel
	objdump -l -S -d --source-comment -M intel $< > $@

$(ROUT)kernel.dump.dat: $(ROUT)kernel
	readelf -a $< > $@

$(RTMP)%.o: %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

$(RTMP)%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

$(ROOT)%/:
	cd $(ROOT) && mkdir -p $*
