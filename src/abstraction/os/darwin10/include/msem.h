#ifndef __MSEM_H
#define __MSEM_H

typedef struct {
  long status;
} __msem_t;

typedef __msem_t __msem_mutex_t;

struct __msem_thrdescr;
typedef struct {
  __msem_t lock;
  struct __msem_thrdescr *waiters;
} __msem_cond_t;

struct timespec;

void __msem_init_global (void);
void __msem_fini_global (void);

void __msem_init (__msem_t *lock, int locked);
void __msem_remove (__msem_t *lock);
int __msem_trylock (__msem_t *lock);
void __msem_lock (__msem_t *lock);
void __msem_unlock (__msem_t * lock);

void __msem_mutex_init (__msem_mutex_t *mtx);
void __msem_mutex_destroy (__msem_mutex_t *mtx);
void __msem_mutex_lock (__msem_mutex_t *mtx);
int __msem_mutex_trylock (__msem_mutex_t *mtx);
void __msem_mutex_unlock (__msem_mutex_t *mtx);

void __msem_cond_init (__msem_cond_t *cv);
void __msem_cond_destroy (__msem_cond_t *cv);
void __msem_cond_wait (__msem_cond_t *cv, __msem_mutex_t *mtx);
int __msem_cond_timedwait (__msem_cond_t *cv, __msem_mutex_t *mtx, const struct timespec *abstime);
void __msem_cond_signal (__msem_cond_t *cv);
void __msem_cond_broadcast (__msem_cond_t *cv);

#endif /* __MSEM_H */
