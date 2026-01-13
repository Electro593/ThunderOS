NAME := kernel
CWD := $(shell pwd)/

RSCRIPTS := $(ROOT)$(SCRIPTS)
RSRC := $(ROOT)$(SRC)
ROUT := $(ROOT)$(OUT)
RTMP := $(ROOT)$(TMP)$(NAME)/

LDS := $(RSCRIPTS)kernel.lds

ASFLAGS := -g -f elf64

# LDFLAGS := -nostdlib -pie
LDFLAGS := -nostdlib -T$(LDS) -pie

CFLAGS := -fshort-wchar -fno-strict-aliasing -ffreestanding -fno-stack-check
CFLAGS += -fno-stack-protector -fomit-frame-pointer -fno-asynchronous-unwind-tables
CFLAGS += -mno-red-zone -fPIE -masm=intel -I$(RSRC) -I$(RSRC)$(NAME) -ggdb3
CFLAGS += $(CWARNS)

CSOURCES := $(wildcard *.c */*.c)
SSOURCES := $(wildcard *.s */*.s)

OBJECTS := $(CSOURCES:%.c=$(RTMP)%.o) $(SSOURCES:%.s=$(RTMP)%.o)
DIRS := $(ROUT) $(patsubst %,$(RTMP)%,$(dir $(CSOURCES) $(SSOURCES)))

.PHONY: kernel

kernel: $(DIRS) $(ROUT)kernel $(ROUT)kernel.dump.asm $(ROUT)kernel.dump.dat

$(ROUT)kernel: $(ROUT)kernel.so
	objcopy $< $@

$(ROUT)kernel.so: $(OBJECTS) $(LDS)
	$(LD) $(LDFLAGS) $(OBJECTS) -o $@

$(ROUT)kernel.dump.asm: $(ROUT)kernel
	objdump -l -S -d --source-comment -M intel $< > $@

$(ROUT)kernel.dump.dat: $(ROUT)kernel
	readelf -a $< > $@

$(RTMP)%.o: %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

$(RTMP)%.o: $(CWD)%.s
	$(AS) $(ASFLAGS) $< -o $@

$(ROOT)%/:
	cd $(ROOT) && mkdir -p $*

$(LDS):
