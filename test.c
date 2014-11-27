#include <msp430g2553.h>
#include <string.h>
#include "threads.h"

#define __delay(val) { \
	volatile int __i; \
	for (__i = 0; __i < (val); __i++); \
}

thread_t tr1, tr2, tr3, tr4;

void	thread1(void)
{
	int x = 0;

	while (1) {
		if (x++ % 2)
			P1OUT |= BIT0;
		else
			P1OUT &= ~BIT0;

		__delay(0xffff);
	}
}

void	thread2(void)
{
	int x = 0;

	while (1) {
		if (x++ % 2)
			P1OUT |= BIT6;
		else
			P1OUT &= ~BIT6;

		__delay(0x1000);

	}
}

void	thread3(void)
{
	while (1) {
		__delay(0xffea);
	}
}

void	thread4(void)
{
	while (1) {
		__delay(0xfea);
	}
}

void	idle(void)
{
	thread_create(&tr1, thread1);
	thread_create(&tr2, thread2);
	thread_create(&tr3, thread3);
	thread_create(&tr4, thread4);
	while (1);
}



void	main(void)
{

	WDTCTL = WDTPW | WDTHOLD;

	P1SEL = 0;
	P1DIR |= BIT0 | BIT6;
	P1OUT &= ~(BIT0 | BIT6);

	threads_init(idle);

	while (1);
}
