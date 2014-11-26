OUT = test

all:
	msp430-gcc -mmcu=msp430g2553 -O3 -c -o timer.o timer.c
	msp430-gcc -mmcu=msp430g2553 -Os -o $(OUT).elf $(OUT).c *.o
