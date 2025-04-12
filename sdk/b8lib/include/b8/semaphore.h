/**
 * @file semaphore.h
 * @brief POSIX-compliant semaphore definitions for the BEEP-8 system.
 * 
 * This file provides POSIX-compliant semaphore definitions and function prototypes 
 * for the BEEP-8 system. It includes a basic `sem_t` structure and functions for 
 * initializing, waiting, posting, and destroying unnamed semaphores.
 * 
 * The provided functions include:
 * - `sem_init`: Initialize an unnamed semaphore
 * - `sem_wait`: Wait on an unnamed semaphore
 * - `sem_timedwait`: Wait on an unnamed semaphore with a timeout
 * - `sem_trywait`: Try to wait on an unnamed semaphore without blocking
 * - `sem_post`: Post (signal) an unnamed semaphore
 * - `sem_getvalue`: Get the current value of an unnamed semaphore
 * 
 * Note that named semaphores are not supported in this implementation. Functions 
 * related to named semaphores, such as `sem_open`, `sem_close`, and `sem_unlink`, 
 * are declared but not implemented and will return errors if used.
 * 
 * Typically, users do not need to use this header directly. It is intended for internal 
 * use within the BEEP-8 system to manage synchronization between threads and processes.
 * 
 * For more detailed information, please refer to the BEEP-8 data sheet.
 */

#pragma once

#ifdef  __cplusplus
extern  "C" {
#endif

#include <stdint.h>
#include <time.h>
typedef struct _sem_t {
  volatile uint32_t sid;
} sem_t;

/**
 * @brief Initialize an unnamed semaphore.
 * 
 * This function initializes an unnamed semaphore. The semaphore can be shared between
 * processes if the `pshared` parameter is non-zero; otherwise, it is shared only between
 * threads of the same process.
 * 
 * @param sem A pointer to the semaphore to initialize.
 * @param pshared A flag indicating whether the semaphore is shared between processes.
 * @param value The initial value of the semaphore.
 * @return 0 on success; -1 on error.
 */
extern int sem_init(sem_t* sem, int pshared, unsigned int value);

/**
 * @brief Wait on an unnamed semaphore.
 * 
 * This function decreases (locks) the semaphore pointed to by `sem`. If the semaphore's
 * value is greater than zero, the decrement proceeds and the function returns immediately.
 * If the semaphore's value is zero, the function blocks until the semaphore's value 
 * becomes greater than zero.
 * 
 * @param sem A pointer to the semaphore to wait on.
 * @return 0 on success; -1 on error.
 */
extern int sem_wait(sem_t* sem);

/**
 * @brief Wait on an unnamed semaphore with a timeout.
 * 
 * This function decreases (locks) the semaphore pointed to by `sem`, but if the 
 * decrement cannot be immediately performed, it will block until the timeout specified
 * by `__abstime` elapses.
 * 
 * @param sem A pointer to the semaphore to wait on.
 * @param __abstime A pointer to a `timespec` structure specifying the timeout.
 * @return 0 on success; -1 on error or timeout.
 */
extern int sem_timedwait(sem_t* sem, const struct timespec* __abstime);

/**
 * @brief Try to wait on an unnamed semaphore without blocking.
 * 
 * This function attempts to decrease (lock) the semaphore pointed to by `sem`. If the 
 * semaphore's value is greater than zero, the decrement proceeds and the function returns 
 * immediately. If the semaphore's value is zero, the function returns immediately without 
 * decrementing the semaphore.
 * 
 * @param sem A pointer to the semaphore to wait on.
 * @return 0 on success; -1 if the semaphore could not be locked immediately.
 */
extern int sem_trywait(sem_t* sem);

/**
 * @brief Post (signal) an unnamed semaphore.
 * 
 * This function increases (unlocks) the semaphore pointed to by `sem`. If the semaphore's 
 * value was zero and there are threads waiting on the semaphore, one of the waiting threads 
 * is awakened.
 * 
 * @param sem A pointer to the semaphore to post.
 * @return 0 on success; -1 on error.
 */
extern int sem_post(sem_t* sem);

/**
 * @brief Get the current value of an unnamed semaphore.
 * 
 * This function stores the current value of the semaphore pointed to by `sem` in the 
 * location pointed to by `sval`.
 * 
 * @param sem A pointer to the semaphore.
 * @param sval A pointer to an integer where the semaphore's value will be stored.
 * @return 0 on success; -1 on error.
 */
extern int sem_getvalue(sem_t* sem, int* sval);


// The following API functions are not supported and cannot be used in the BEEP-8 system.
#define SEM_FAILED	((sem_t *) 0)
extern int    sem_destroy(sem_t* sem);
extern sem_t* sem_open (const char* name, int __oflag, ...);
extern int    sem_close (sem_t* sem);
extern int    sem_unlink(const char *name);

#ifdef  __cplusplus
}
#endif