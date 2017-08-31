TOP_DIR= /home/sec/kvm-side-channel-attack/side_channel

INCLUDE= $(TOP_DIR)/include
INCLUDE_SOURCE=
LIBINCLUDE=$(TOP_DIR)/lib_src

INCLUDE_SOURCE+=$(INCLUDE)/mem.c

INCLUDE_SOURCE+=$(INCLUDE)/tools.c

CFLAGS= -I. -I$(INCLUDE) -I$(LIBINCLUDE)
