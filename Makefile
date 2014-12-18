TOOLCHAIN :=
FLAGS :=
THREAD_SIZE :=
ARCH_SPECIFIC :=

.SUFFIXES:

% : RCS/%,v
% : RCS/%
% : %,v
% : s.%
% : SCCS/s.%

DIR := arch/$(ARCH)
OUT_DIR := out
INC := -I$(DIR) -Iinclude

OUT := threads
OUT_H := $(OUT_DIR)/$(OUT).h
OUT_A := $(OUT_DIR)/$(OUT).a
OUT_O := $(OUT).o
OUT_C := $(OUT).c
OUT_DEPS := $(OUT).c $(DIR)/platform.h include/common.h include/arch_api.h

ARCH_OUT := arch.a
CONFIG_MK := $(DIR)/config.mk

ifeq ($(ARCH),)
  $(error ARCH not specified)
endif

ifeq ($(wildcard $(DIR)),)
  $(error Missing direcotry $(DIR))
endif

ifeq ($(wildcard $(CONFIG_MK)),)
  $(error Missing $(CONFIG_MK))
endif

# Load arch configuration
include $(CONFIG_MK)

ifeq ($(TOOLCHAIN),)
  $(error Missing TOOLCHAIN setting in $(CONFIG_MK))
endif

ifeq ($(THREAD_SIZE),)
  $(error Missing THREAD_SIZE setting in $(CONFIG_MK))
endif

ifeq ($(NR_REGS),)
  $(error Missing NR_REGS setting in $(CONFIG_MK))
endif

ifeq ($(REG_TYPE),)
  $(error Missing REG_TYPE setting in $(CONFIG_MK))
endif

ifneq ($(wildcard $(DIR)/Makefile),)
  ARCH_SPECIFIC := $(DIR)/$(ARCH_OUT)
endif

CONFIG := -DTHREAD_SIZE=$(THREAD_SIZE) \
	-DNR_REGS=$(NR_REGS) \
	-DREG_TYPE='$(REG_TYPE)'

ARCH_CONFIG := ARCH=$(ARCH) CPU=$(CPU) TOOLCHAIN=$(TOOLCHAIN) \
	CC="$(TOOLCHAIN)-gcc $(FLAGS) $(CONFIG) -I../../include" \
	AR=$(TOOLCHAIN)-ar \
	LD=$(TOOLCHAIN)-ld \
	AS=$(TOOLCHAIN)-as \
	CPP=$(TOOLCHAIN)-cpp \
	OUT=$(ARCH_OUT)

##########################################################

$(OUT_A): $(OUT_DIR) $(OUT_O) $(ARCH_SPECIFIC)
	$(TOOLCHAIN)-ar cr $@ $(OUT_O) $(ARCH_SPECIFIC)

$(OUT_O): $(OUT_H) $(OUT_DEPS)
	$(TOOLCHAIN)-gcc $(FLAGS) $(CONFIG) -Wall $(INC) -O3 -c -o $@ $(OUT).c

$(OUT_H): include/$(OUT).h.in
	$(shell sed -e s/@THREAD_SIZE@/$(THREAD_SIZE)/g $< > $@)

$(ARCH_SPECIFIC):
	make -C $(DIR) $(ARCH_CONFIG) $(ARCH_OUT) 

$(OUT_DIR):
	mkdir -p $@

.PHONY: clean
clean:
	rm -f *.o
	rm -f $(DIR)/*.o
	rm -f $(DIR)/*.a
	rm -rf $(OUT_DIR)
