#ifndef THREADS_H
#define THREADS_H

#define THREAD_SIZE		64
#define THREAD_STACK_SIZE	(THREAD_SIZE) / 2

#define NR_REGS	13

typedef union thread {
	struct {
		unsigned int regs[NR_REGS];
		union thread *next;
		union thread *prev;
	};
	unsigned int stack[THREAD_STACK_SIZE];
} thread_t;

extern thread_t *current;

extern void thread_create(thread_t *t, void *fn);
extern void thread_exit(void);
extern void threads_init(void (*fn)(void));
extern void thread_sleep(thread_t **wq, thread_t *t);
extern void thread_wakeup(thread_t **wq);

#define DEFINE_WAIT_QUEUE(q) thread_t *q = (void *)0

#define thread_wait(wq, cond) \
	while (!(cond)) { thread_sleep((wq), current); }

#endif /* THREADS_H */
