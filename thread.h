#define THREADS_MAX	3
#define TR_NEW		0x01

typedef union {
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
	unsigned int flags;
	unsigned int stack[32];
} thread_t;
