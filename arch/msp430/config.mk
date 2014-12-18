TOOLCHAIN := msp430

# Defaults for all msp430
THREAD_SIZE := 64
NR_REGS := 13
REG_TYPE := unsigned int

ifeq ($(CPU),g2553)
  FLAGS := -mmcu=msp430g2553
endif
