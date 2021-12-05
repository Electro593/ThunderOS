ifeq ($(DEBUG),)
TARGET = ./build/ThunderOS.efi
SRCS = ./src/kernel/entry.c
else
TARGET = ./build/ThunderOS_Debug.efi
SRCS = ./src/kernel/entry.c
CFLAGS = -ggdb
SECTIONS = -j .debug_info -j .debug_abbrev -j .debug_loc -j .debug_aranges -j .debug_line -j .debug_macinfo -j .debug_str
# -j .debug_ranges
endif

USE_GCC=1
include uefi/Makefile