#ifndef THREADS_H
#define THREADS_H

#define THREAD_SIZE		64
#define THREAD_STACK_SIZE	(THREAD_SIZE) / 2

struct cpu_context {
	unsigned int sp;
	unsigned int r4;
	unsigned int r5;
	unsigned int r6;
	unsigned int r7;
	unsigned int r8;
	unsigned int r9;
	unsigned int r10;
	unsigned int r11;
	unsigned int r12;
	unsigned int r13;
	unsigned int r14;
	unsigned int r15;
	unsigned int next;
};

typedef union {
//typedef struct bla {
	struct cpu_context ctx;
//	unsigned int sp;
//	unsigned int r4;
//	unsigned int r5;
//	unsigned int r6;
//	unsigned int r7;
//	unsigned int r8;
//	unsigned int r9;
//	unsigned int r10;
//	unsigned int r11;
//	unsigned int r12;
//	unsigned int r13;
//	unsigned int r14;
//	unsigned int r15;
//	unsigned int next;

	unsigned int stack[THREAD_STACK_SIZE];
} thread_t;

extern	void	thread_create(thread_t *t, void *fn);
extern	void	threads_init(void (*fn)(void));

#endif
