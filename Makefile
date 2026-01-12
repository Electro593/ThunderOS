export AS := nasm
export CC := gcc
export LD := ld

export ARCH := $(shell uname -m | sed s,i[3456789]86,ia32,)
ifeq ($(ARCH), aarch64)
   export EFIARCH := pei-aarch64-little
else
   export EFIARCH := efi-app-$(ARCH)
endif

export OUT := build/
export SRC := src/
export SCRIPTS := scripts/

CWARNS := -Wall -Wextra -Werror
CWARNS += -Wno-unused-function -Wno-unused-variable -Wno-unused-parameter
CWARNS += -Wno-unused-but-set-variable -Wno-missing-braces -Wno-sign-compare
CWARNS += -Wno-pointer-sign -Wno-unused-value -Wno-builtin-declaration-mismatch
CWARNS += -Wno-unterminated-string-initialization
export CWARNS

.PHONY: mount all loader kernel clean

mount: all
	./scripts/mount.sh

all: loader kernel

loader:
	cd src/loader && $(MAKE)

kernel:
	cd src/kernel && $(MAKE)

clean:
	rm -rf $(OUT)
