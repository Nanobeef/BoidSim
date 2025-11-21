#include "basic.h"

#include <semaphore.h>
#include <sched.h> // cpu_set_t
#include <unistd.h>
#include <signal.h>
#include <semaphore.h>
#include <pthread.h>


Thread main_thread = {0};

thread_local Thread *THREAD = NULL;
u32 THREAD_COUNT = 0;
Thread *MAIN_THREAD = NULL;
	

Arena main_arena = {0};



void init_threads()
{
	THREAD = &main_thread;	
	THREAD->arena = allocate_sub_arena(MB * 128, &main_arena);
	MAIN_THREAD = THREAD;
	THREAD_COUNT = get_physical_thread_count();
}
void cleanup_threads()
{
	
}


u32 get_physical_thread_count()
{
	cpu_set_t mask;
	sched_getaffinity(0,sizeof(mask), &mask);
	s32 count = CPU_COUNT(&mask);
	return defmin((u32)count, 1024);
}


typedef struct{
	pthread_t thread;
}ThreadHandle;

void *start_routine(void *arg)
{
	Thread *thread = arg;
	THREAD = thread;
	return ((pfn_thread)(thread->pfn))(thread);
}

Thread* start_thread(pfn_thread pfn, void *parameters, u64 arena_size, Arena *arena)
{
	ThreadHandle *handle = arena_alloc(sizeof(ThreadHandle), 1,0,arena);
	Thread *thread = arena_alloc(sizeof(Thread), 0,0, arena);
	*thread = (Thread){
		.arena = allocate_sub_arena(arena_size, arena),
		.parameters = parameters,		
		.handle = handle,
		.pfn = pfn,
	};
	pthread_create(&handle->thread,NULL, start_routine, thread);
	return thread;
}

b32 set_thread_affinity(Thread *thread, u32 cpu_index)
{
	ThreadHandle *handle = thread->handle;
	cpu_set_t set;
	CPU_ZERO(&set);
	CPU_SET((u64)cpu_index, &set);
	if(pthread_setaffinity_np(handle->thread, sizeof(cpu_set_t), &set))
	{
		return false;
	}
	return true;
}

void* join_thread(Thread *thread)
{
	ThreadHandle *handle = thread->handle;
	void *ret = 0;
	pthread_join(handle->thread, &ret);
	return ret;
}

Mutex create_mutex(Arena* arena)
{
	pthread_mutex_t* handle = arena_alloc(sizeof(pthread_mutex_t), 0,0,arena);
	pthread_mutex_init(handle, NULL);
	Mutex mtx = {
		.handle = (void*)handle,
	};
	return mtx;
}
void destroy_mutex(Mutex mtx)
{
	pthread_mutex_destroy((pthread_mutex_t*)(mtx.handle));
}
Mutex mutex_lock(Mutex mtx)
{
	pthread_mutex_t* handle = mtx.handle;
	pthread_mutex_lock(handle);
	return mtx;
}
Mutex mutex_unlock(Mutex mtx)
{
	pthread_mutex_t* handle = mtx.handle;
	pthread_mutex_unlock(handle);
	return mtx;
}

// MUTEX ========================= MUTEX
// COND ========================== COND

Cond create_cond(Arena* arena)
{
	pthread_cond_t* handle = arena_alloc(sizeof(pthread_cond_t), 0,0,arena);
	pthread_cond_init(handle, NULL);
	Cond cond = {
		.handle = handle,
	};
	return cond;
}
void destroy_cond(Cond cond)
{
	pthread_cond_t* handle = cond.handle;
	pthread_cond_destroy(handle);
}

Cond cond_wait(Cond cond, Mutex mutex)
{
	pthread_mutex_t* mutex_handle = mutex.handle;
	pthread_cond_t* cond_handle = cond.handle;
	pthread_cond_wait(cond_handle, mutex_handle);
	return cond;
}
Cond cond_timedwait(Cond cond, Mutex mutex, u64 ns)
{
	pthread_mutex_t* mutex_handle = mutex.handle;
	pthread_cond_t* cond_handle = cond.handle;
	struct timespec ts = {.tv_nsec = ns % 1000000000, .tv_sec = ns / 1000000000};
	pthread_cond_timedwait(cond_handle, mutex_handle, &ts);
	return cond;
}
Cond cond_signal(Cond cond)
{
	pthread_cond_t* cond_handle = cond.handle;
	pthread_cond_signal(cond_handle);
	return cond;
}
Cond cond_broadcast(Cond cond)
{
	pthread_cond_t* cond_handle = cond.handle;
	pthread_cond_broadcast(cond_handle);
	return cond;
}

// COND =========================== COND
// SEMAPHORE ====================== SEMAPHORE


Semaphore create_semaphore(Arena* arena)
{
	sem_t* handle = arena_alloc(sizeof(sem_t),0,0, arena);
	sem_init(handle, 0, 0); 
	Semaphore semaphore = {handle};
	return semaphore;
}
void destroy_semaphore(Semaphore semaphore)
{
	sem_t* handle = semaphore.handle;
	sem_destroy(handle);	
}
Semaphore semaphore_signal(Semaphore semaphore)
{
	sem_t* handle = semaphore.handle;
	if(sem_post(handle))
	{
		printf("Semaphore post error\n");
	}
	return semaphore;
}

Semaphore semaphore_wait(Semaphore semaphore)
{
	sem_t* handle = semaphore.handle;
	if(sem_wait(handle))
	{
		printf("Semaphore wait error\n");
	}
	return semaphore;
}

// SEMAPHORE =============================== SEMAPHORE
// BARRIER ================================= BARRIER

Barrier create_barrier(u32 count, Arena* arena)
{
	pthread_barrier_t* handle = arena_alloc(sizeof(pthread_barrier_t),0,0, arena);
	pthread_barrier_init(handle, NULL, count);
	Barrier barrier = {
		.handle = (void*)handle,	
	};
	return barrier;
}

void destroy_barrier(Barrier barrier)
{
	pthread_barrier_destroy((pthread_barrier_t*)barrier.handle);
}

b32 barrier_wait(Barrier barrier)
{
	int ret = pthread_barrier_wait((pthread_barrier_t*)barrier.handle);
	return (ret == PTHREAD_BARRIER_SERIAL_THREAD);
}
