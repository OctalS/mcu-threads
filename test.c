#include <msp430g2553.h>
#include <string.h>
#include "thread.h"

#define TIMER_TICK	3000

thread_t threads[THREADS_MAX];

thread_t *current = &threads[0];

#define __delay(val) { \
	volatile int __i; \
	for (__i = 0; __i < (val); __i++); \
}

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

void	idle_thread(void)
{
	while (1);
}

void	__add_thread(int idx, void *fn)
{
	//memset(&threads[idx].stack, 0x5a, sizeof(threads[idx].stack));

	threads[idx].stack[31] = (unsigned int)fn;
	threads[idx].stack[30] = GIE;
	threads[idx].sp = (unsigned int)&threads[idx] + 60;
}

void	enable_timer(void)
{

	TACTL = TACLR;
	TACCTL0 = CCIE;
	TACCR0 = TIMER_TICK;
	TACTL = TASSEL_2 | ID_3 | MC_1;

	__enable_interrupt();
}


void	main(void)
{

	WDTCTL = WDTPW | WDTHOLD;

	P1SEL = 0;
	P1DIR |= BIT0 | BIT6;
	P1OUT &= ~(BIT0 | BIT6);

	__add_thread(1, thread1);
	__add_thread(2, thread2);
	enable_timer();

	while (1);
}
