#include <msp430g2553.h>
#include <isr_compat.h>
#include "threads.h"

#define TIMER_TICK	1000

#define PC_OFF		((THREAD_STACK_SIZE) - 1)
#define SR_OFF		((THREAD_STACK_SIZE) - 2)
#define SP_OFF		((THREAD_SIZE) - 4)

/* This always points to the currently running thread */
thread_t *current;

/* List of all running threads */
thread_t *running;

static thread_t thread0;
static unsigned int save_r14;

/* threads scheduler */
ISR(TIMER0_A0, timerA_isr)
{
	__asm__ __volatile__ (

"	cmp #0, %2\n"		/* if timer is disabled do nothing */
"	jeq 1f\n"

"	mov r14, %1\n"
"	mov %0, r14\n"		/* get current */

"	mov r1, 0(r14)\n"	/* save current stack */
"	mov r4, 2(r14)\n"	/* save rest of cpu regs */
"	mov r5, 4(r14)\n"
"	mov r6, 6(r14)\n"
"	mov r7, 8(r14)\n"
"	mov r8, 10(r14)\n"
"	mov r9, 12(r14)\n"
"	mov r10, 14(r14)\n"
"	mov r11, 16(r14)\n"
"	mov r12, 18(r14)\n"
"	mov r13, 20(r14)\n"
"	mov %1, 22(r14)\n"	/* save original r14 */
"	mov r15, 24(r14)\n"

"	mov 26(r14), r15\n"	/* get next thread */
"	mov r15, %0\n"		/* current = next */

"	mov 0(r15), r1\n"	/* get next stack */
"	mov 2(r15), r4\n"	/* restore next's context */
"	mov 4(r15), r5\n"
"	mov 6(r15), r6\n"
"	mov 8(r15), r7\n"
"	mov 10(r15), r8\n"
"	mov 12(r15), r9\n"
"	mov 14(r15), r10\n"
"	mov 16(r15), r11\n"
"	mov 18(r15), r12\n"
"	mov 20(r15), r13\n"
"	mov 22(r15), r14\n"
"	mov 24(r15), r15\n"
"1:\n"

	: "+m" (current), "+m" (save_r14)
	: "m" (TACCTL0)

	);
}

/* This yelds the cpu to another thread.
 * As there is no sw int instruction,
 * we simulate one */
#define thr_sched()		\
	asm (			\
"	dint\n"			\
"	nop\n"			\
"	push.w #1f\n"		\
"	push.w r2\n"		\
"	bis #8, @r1\n"		\
"	jmp timerA_isr\n"	\
"1:	nop\n"			\
	)

#define thr_lonely(t) ((t)->next == (t))

#define thr_spin() { while(1); }

/* Disables timer A
 *
 * The nop is for safety an interrupt
 * rises while still disalbeing the timer */
#define thr_lock()		\
	do {			\
		TACCTL0 = 0;	\
		nop();		\
	} while (0)

/* Reenables timer A */
#define thr_unlock()		\
	do {			\
		TACCTL0 = CCIE;	\
	} while (0)

/* Adds thread 't' after thread 'where' */
static inline void __add_thread(thread_t *where, thread_t *t)
{
	thread_t *next;

	next = where->next;

	where->next = t;
	t->prev = where;
	t->next = next;
	next->prev = t;
}

/* Unlinks thread 't' from the list
 * of running threads.
 * NOTE: t->next, and t->prev are left untouched */
static inline void del_thread(thread_t *t)
{
	thread_t *prev, *next;

	prev = t->prev;
	next = t->next;
	prev->next = next;
	next->prev = prev;

	if (running == t)
		running = next;
}

#define add_first_thread(tr)		\
	do {				\
		(tr)->next = (tr);	\
		(tr)->prev = (tr);	\
		running = (tr);		\
	} while (0)

#define add_thread_after(where, tr)	__add_thread((where), (tr))
#define add_thread_before(where, tr)	__add_thread((where)->prev, (tr))

static inline void run_thread(thread_t *t)
{
	if (!running)
		add_first_thread(t);
	else
		add_thread_before(running, t);
}

/* Adds a freshly created thread */
static inline void add_thread(thread_t *t, void *fn)
{
	t->stack[PC_OFF] = (unsigned int)fn;
	t->stack[SR_OFF] = GIE;
	t->regs[0] = (unsigned int)t + SP_OFF;

	run_thread(t);
}

/* Unlink thread 't' from the list
 * of running threads and put it
 * in the list of sleeping ones */
void thread_sleep(thread_t **wq, thread_t *t)
{
	thread_t *l;

	thr_lock();
	del_thread(t);

	if (!*wq) {
		t->prev = t;
		*wq = t;
		goto sched;
	}

	l = (*wq)->prev;
	(*wq)->prev = t;
	l->next = t;
	t->prev = l;
sched:
	thr_unlock();
	thr_sched();
}

/* Wakes up all threads in the queue 'wq' immediatly.
 * Threads are appended right after current and 
 * and thread switch is performed. */
void thread_wakeup_now(thread_t **wq)
{
	thread_t *rnext, *slast;

	thr_lock();

	if (!*wq)
		goto exit;

	rnext = current->next;
	slast = (*wq)->prev;
	current->next = *wq;
	(*wq)->prev = current;
	slast->next = rnext;
	rnext->prev = slast;

	*wq = (void *)0;

	thr_unlock();
	thr_sched();
	return;
exit:
	thr_unlock();
}

/* Wakes up all threads in the queue 'wq'.
 * Threads are appended at the end of
 * the list of running threads */
void thread_wakeup(thread_t **wq)
{
	thread_t *rlast, *slast;

	thr_lock();

	if (!*wq)
		goto exit;

	rlast = running->prev;
	slast = (*wq)->prev;
	rlast->next = (*wq);
	(*wq)->prev = rlast;
	slast->next = running;
	running->prev = slast;

	*wq = (void *)0;
exit:
	thr_unlock();
}

void thread_create(thread_t *t, void *fn)
{
	thr_lock();
	add_thread(t, fn);
	thr_unlock();
}

/* Permanently unlink current thread from
 * the list of running threads. */
void thread_exit(void)
{
	thr_lock();

	if (thr_lonely(current))
		thr_spin();

	del_thread(current);
	thr_unlock();
	thr_sched();
}

void threads_init(void (*fn)(void))
{
	__disable_interrupt();

	add_first_thread(&thread0);
	current = &thread0;

	/* setup Timer A */
	TACTL = TACLR;
	TACCTL0 = CCIE;
	TACCR0 = TIMER_TICK;
	TACTL = TASSEL_2 | ID_0 | MC_1;

	__enable_interrupt();

	/* Enter thread 0 immediately before first timer tick! */
	(*fn)();
}
