#ifndef THREADS_API
#define THREADS_API

#define thr_lock()	arch_threads_lock()
#define thr_unlock()	arch_threads_unlock()
#define thr_sched()	arch_schedule()
#define disable_interrupts() arch_disable_interrupts();
#define enable_interrupts() arch_enable_interrupts();

static void arch_init(void);
static void arch_prepare_thread(thread_t *t, void *fn);

#endif /* THREADS_API */
