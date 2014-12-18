#ifndef COMMON_H
#define COMMON_H

#if ((__GNUC__ >= 4) && (__GNUC_MINOR__ >= 6))

#define STACK_MIN	16
#define THREAD_SIZE_MIN	(NR_REGS * sizeof(REG_TYPE) + 2 * sizeof(void *) + STACK_MIN)

_Static_assert(!(THREAD_SIZE < THREAD_SIZE_MIN), "THREAD_SIZE is too small. Wont compile.");

#endif

typedef REG_TYPE reg_t;

typedef union thread {
	struct {
		reg_t regs[NR_REGS];
		union thread *next;
		union thread *prev;
	};
	unsigned char stack[THREAD_SIZE];
} thread_t;

extern thread_t *current;
extern thread_t *running;

#endif /* COMMON_H */
