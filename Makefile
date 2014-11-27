OUT = test

all:
	msp430-gcc -mmcu=msp430g2553 -O3 -c -o threads.o threads.c
	msp430-gcc -mmcu=msp430g2553 -Os -o $(OUT).elf $(OUT).c *.o
