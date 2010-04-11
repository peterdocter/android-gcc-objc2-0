/***********************************************************************
* objc-os.h
* OS portability layer.
**********************************************************************/

#ifndef _OBJC_OS_H
#define _OBJC_OS_H

#include <pthread.h>

typedef struct {
	pthread_mutex_t mutex;
	pthread_cond_t readWait;
	unsigned int readersActive;
	unsigned int readersWaiting;
	pthread_cond_t writeWait;
	unsigned int writersActive;
	unsigned int writersWaiting;
	unsigned int valid;
} rwlock_t;

#define RWLOCK_VALID    0x0decaf

#define RWLOCK_INITIALIZER \
{ \
PTHREAD_MUTEX_INITIALIZER, \
PTHREAD_COND_INITIALIZER, 0, 0 \
PTHREAD_COND_INITIALIZER, 0, 0 \
RWLOCK_VALID, \
}

typedef enum {
	rwlock_readmode,
	rwlock_writemode
} rwlock_mode;

extern int rwlock_initalize(rwlock_t *lock);
extern int rwlock_destroy(rwlock_t *lock);
extern int rwlock_lock(rwlock_t *lock, rwlock_mode mode);
extern int rwlock_trylock(rwlock_t *lock, rwlock_mode mode);
extern int rwlock_unlock(rwlock_t *lock, rwlock_mode mode);

#endif
