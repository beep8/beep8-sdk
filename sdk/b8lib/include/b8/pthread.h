/**
 * @file pthread.h
 * @brief Header file for POSIX thread (pthread) functions.
 *
 * This header file provides the declarations for various POSIX thread (pthread) functions.
 * It includes functions for creating and managing threads, setting and getting thread attributes,
 * and handling thread scheduling and synchronization. The functionality of these functions
 * varies based on the specific OS environment.
 *
 * - The following functions are fully supported and work correctly in this BEEP-8 environment:
 *   - pthread_create
 *   - pthread_yield
 *   - pthread_self
 *   - pthread_attr_init
 *   - pthread_attr_destroy
 *   - pthread_attr_setstacksize
 *   - pthread_attr_getstacksize
 *   - pthread_attr_setstack
 *   - pthread_attr_getstack
 *   - pthread_attr_getdetachstate
 *   - pthread_attr_setdetachstate
 *
 * - The following functions are not supported in this OS environment and always return -ERRNOSYS:
 *   - pthread_getschedparam
 *   - pthread_setschedparam
 *   - pthread_detach
 *   - pthread_join
 *   - pthread_cancel
 *   - pthread_setcanceltype
 *   - pthread_testcancel
 *
 * - The following functions can be called and will set attributes, but the actual scheduling
 *   policy or affinity settings are ignored in this BEEP-8 environment, making them effectively unsupported:
 *   - pthread_attr_setschedpolicy
 *   - pthread_attr_getschedpolicy
 *   - pthread_attr_setschedparam
 *   - pthread_attr_getschedparam
 *   - pthread_attr_setinheritsched
 *   - pthread_attr_getinheritsched
 *   - pthread_attr_setaffinity_np
 *   - pthread_attr_getaffinity_np
 */
#pragma once

#ifdef  __cplusplus
extern  "C" {
#endif

#include <b8/os.h>
#include <b8/type.h>
#include <b8/sched.h>

#undef  pthread_attr_t
#define pthread_attr_t b8_pthread_attr_t

typedef b8OsPid pthread_t;

typedef void *pthread_addr_t;

typedef pthread_addr_t (*pthread_startroutine_t)(pthread_addr_t);
typedef pthread_startroutine_t pthread_func_t;

typedef struct _b8_pthread_attr_t {
  void*   stackaddr;    // Address of memory to be used as stack
  size_t  stacksize;    // Size of the stack allocated for the pthread
  u8      policy;
  u8      detachstate;  // Initialize to the detach state
  s16     priority;     // The actual scheduling policy is ignored and thus effectively unsupported.
  u16     irq_no;
} b8_pthread_attr_t;

#define PTHREAD_STACK_MIN             (0x200)

#undef  PTHREAD_CREATE_JOINABLE
#define PTHREAD_CREATE_JOINABLE       0

#undef  PTHREAD_CREATE_DETACHED
#define PTHREAD_CREATE_DETACHED       1

#define pthread_equal(t1,t2) ((t1) == (t2))

/**
 * @brief Creates a new thread.
 *
 * This function is part of the POSIX standard and is used to create a new thread. It initializes a new thread
 * and makes it executable, running the specified start routine with the given argument.
 * This function works correctly in this OS environment.
 *
 * @param thread A pointer to the thread identifier that will be returned.
 * @param attr A pointer to the thread attributes object that specifies attributes for the new thread. If NULL, default attributes are used.
 * @param startroutine The routine that the new thread will execute.
 * @param arg The argument passed to the start routine.
 * @return 0 on success, or an error code on failure.
 */
extern int pthread_create(
  pthread_t* thread,
  const pthread_attr_t* attr,
  pthread_startroutine_t startroutine,
  pthread_addr_t arg
);

/**
 * @brief Yields the processor to another thread.
 *
 * This function is part of the POSIX standard and is used to yield the processor from the calling thread to another thread,
 * allowing other threads to run. This function works correctly in this OS environment.
 *
 * @return 0 on success, or an error code on failure.
 */
extern int pthread_yield(void);

/**
 * @brief Returns the thread identifier of the calling thread.
 *
 * This function is part of the POSIX standard and is used to obtain the thread identifier of the calling thread.
 * This function works correctly in this OS environment.
 *
 * @return The thread identifier of the calling thread.
 */
extern pthread_t pthread_self(void);

/**
 * @brief Initializes a thread attributes object.
 *
 * This function is part of the POSIX standard and is used to initialize a thread attributes object with default values.
 * This function works correctly in this OS environment.
 *
 * @param attr A pointer to the thread attributes object to be initialized.
 * @return 0 on success, or an error code on failure.
 */
extern int pthread_attr_init(pthread_attr_t* attr);

/**
 * @brief Destroys a thread attributes object.
 *
 * This function is part of the POSIX standard and is used to destroy a thread attributes object,
 * freeing any resources it may hold. This function works correctly in this OS environment.
 *
 * @param attr A pointer to the thread attributes object to be destroyed.
 * @return 0 on success, or an error code on failure.
 */
extern int pthread_attr_destroy(pthread_attr_t *attr);

/**
 * @brief Sets the stack size attribute in the thread attributes object.
 *
 * This function is part of the POSIX standard and is used to set the stack size attribute
 * in a thread attributes object. This function works correctly in this OS environment.
 *
 * @param attr A pointer to the thread attributes object.
 * @param stacksize The stack size to be set.
 * @return 0 on success, or an error code on failure.
 */
extern int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize);

/**
 * @brief Gets the stack size attribute from the thread attributes object.
 *
 * This function is part of the POSIX standard and is used to retrieve the stack size attribute
 * from a thread attributes object. This function works correctly in this OS environment.
 *
 * @param attr A pointer to the thread attributes object.
 * @param stacksize A pointer to a variable where the stack size will be stored.
 * @return 0 on success, or an error code on failure.
 */
extern int pthread_attr_getstacksize(const pthread_attr_t *attr, size_t *stacksize);

/**
 * @brief Sets the stack address and size attributes in the thread attributes object.
 *
 * This function is part of the POSIX standard and is used to set the stack address and size
 * attributes in a thread attributes object. This function works correctly in this OS environment.
 *
 * @param attr A pointer to the thread attributes object.
 * @param stackaddr The stack address to be set.
 * @param stacksize The stack size to be set.
 * @return 0 on success, or an error code on failure.
 */
extern int pthread_attr_setstack(pthread_attr_t *attr, void *stackaddr, long stacksize);

/**
 * @brief Gets the stack address and size attributes from the thread attributes object.
 *
 * This function is part of the POSIX standard and is used to retrieve the stack address and size
 * attributes from a thread attributes object. This function works correctly in this OS environment.
 *
 * @param attr A pointer to the thread attributes object.
 * @param stackaddr A pointer to a variable where the stack address will be stored.
 * @param stacksize A pointer to a variable where the stack size will be stored.
 * @return 0 on success, or an error code on failure.
 */
extern int pthread_attr_getstack(const pthread_attr_t *attr, void **stackaddr, long *stacksize);

/**
 * @brief Gets the detach state attribute from the thread attributes object.
 *
 * This function is part of the POSIX standard and is used to retrieve the detach state attribute
 * from a thread attributes object. This function works correctly in this OS environment.
 *
 * @param attr A pointer to the thread attributes object.
 * @param detachstate A pointer to a variable where the detach state will be stored.
 * @return 0 on success, or an error code on failure.
 */
extern int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *detachstate);

/**
 * @brief Sets the detach state attribute in the thread attributes object.
 *
 * This function is part of the POSIX standard and is used to set the detach state attribute
 * in a thread attributes object. This function works correctly in this OS environment.
 *
 * @param attr A pointer to the thread attributes object.
 * @param detachstate The detach state to be set.
 * @return 0 on success, or an error code on failure.
 */
extern int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate);

/**
 * @brief Sets the scheduling parameters in the thread attributes object.
 *
 * This function is part of the POSIX standard and allows setting the scheduling parameters
 * in the thread attributes object. However, in this specific OS environment, while the function
 * can be called and the parameters can be set, the actual scheduling policy is ignored, making
 * it effectively unsupported.
 *
 * @param attr A pointer to the thread attributes object.
 * @param param A pointer to a struct sched_param containing the scheduling parameters.
 * @return 0 indicating success, but note that the scheduling policy is ignored.
 */
extern int pthread_attr_setschedparam(pthread_attr_t *attr, const struct sched_param *param);

/**
 * @brief Retrieves the scheduling parameters from the thread attributes object.
 *
 * This function is part of the POSIX standard and allows retrieving the scheduling parameters
 * from the thread attributes object. However, in this specific OS environment, while the function
 * can be called and the parameters can be retrieved, the actual scheduling policy is ignored, making
 * it effectively unsupported.
 *
 * @param attr A pointer to the thread attributes object.
 * @param param A pointer to a struct sched_param where the scheduling parameters will be stored.
 * @return 0 indicating success, but note that the retrieved scheduling policy is ignored.
 */
extern int pthread_attr_getschedparam(const pthread_attr_t *attr, struct sched_param *param);

/**
 * @brief Retrieves the scheduling policy and parameters of the specified thread.
 *
 * This function is part of the POSIX standard. However, in this specific environment,
 * it is not supported and will always return -ERRNOSYS.
 *
 * @param thread The thread whose scheduling parameters are to be retrieved.
 * @param policy A pointer to an integer where the policy will be stored.
 * @param param A pointer to a struct sched_param where the scheduling parameters will be stored.
 * @return -ERRNOSYS as this function is not supported in this environment.
 */
extern int pthread_getschedparam(pthread_t thread, int* policy, struct sched_param* param);

/**
 * @brief Sets the scheduling policy and parameters of the specified thread.
 *
 * This function is part of the POSIX standard. However, in this specific environment,
 * it is not supported and will always return -ERRNOSYS.
 *
 * @param thread The thread whose scheduling parameters are to be set.
 * @param policy The new scheduling policy.
 * @param param A pointer to a struct sched_param containing the new scheduling parameters.
 * @return -ERRNOSYS as this function is not supported in this environment.
 */
extern int pthread_setschedparam(pthread_t thread, int policy, const struct sched_param* param);

/**
 * @brief Sets the scheduling policy attribute in the thread attributes object.
 *
 * This function is part of the POSIX standard. In this specific environment, while the function can be called and 
 * the policy can be set, the actual scheduling policy is ignored and thus effectively unsupported.
 *
 * @param attr A pointer to the thread attributes object.
 * @param policy The new scheduling policy.
 * @return 0 indicating success, but note that the policy is ignored.
 */
extern int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy);

/**
 * @brief Retrieves the scheduling policy from the thread attributes object.
 *
 * This function is part of the POSIX standard. In this specific environment, while the function can be called and 
 * the policy can be retrieved, the actual scheduling policy is ignored and thus effectively unsupported.
 *
 * @param attr A pointer to the thread attributes object.
 * @param policy A pointer to an integer where the policy will be stored.
 * @return 0 indicating success, but note that the retrieved policy is ignored in practice.
 */
extern int pthread_attr_getschedpolicy(const pthread_attr_t *attr, int *policy);

/**
 * @brief Terminates the calling thread.
 *
 * This function is part of the POSIX standard and allows for the termination of the calling thread.
 * However, in this specific OS environment, thread termination is not supported. While the function
 * can be called, it will only put the thread into an indefinite wait state, and the thread will not
 * be released or cleaned up.
 *
 * @param value The exit status of the thread.
 */
extern  void pthread_exit(pthread_addr_t value);

/**
 * @brief Detaches the specified thread.
 *
 * This function is part of the POSIX standard and is used to detach a thread, allowing its resources
 * to be automatically reclaimed upon termination. However, in this OS environment, this function is not supported
 * and always returns -ERRNOSYS.
 *
 * @param thread The thread to be detached.
 * @return -ERRNOSYS indicating that this function is not supported.
 */
extern int pthread_detach(pthread_t thread);

/**
 * @brief Waits for the specified thread to terminate.
 *
 * This function is part of the POSIX standard and is used to wait for a thread to terminate and optionally
 * retrieve its exit status. However, in this OS environment, this function is not supported and always returns -ERRNOSYS.
 *
 * @param thread The thread to wait for.
 * @param value A pointer to a location where the exit status of the thread will be stored.
 * @return -ERRNOSYS indicating that this function is not supported.
 */
extern int pthread_join(pthread_t thread, pthread_addr_t *value);

/**
 * @brief Cancels the specified thread.
 *
 * This function is part of the POSIX standard and is used to request the cancellation of a thread. However, in this OS environment,
 * this function is not supported and always returns -ERRNOSYS.
 *
 * @param thread The thread to be canceled.
 * @return -ERRNOSYS indicating that this function is not supported.
 */
extern int pthread_cancel(pthread_t thread);

/**
 * @brief Sets the cancelability type of the calling thread.
 *
 * This function is part of the POSIX standard and is used to set the cancelability type (deferred or asynchronous) of the calling thread.
 * However, in this OS environment, this function is not supported and always returns -ERRNOSYS.
 *
 * @param type The new cancelability type.
 * @param oldtype A pointer to an integer where the previous cancelability type will be stored.
 * @return -ERRNOSYS indicating that this function is not supported.
 */
extern int pthread_setcanceltype(int type, int *oldtype);

/**
 * @brief Creates a cancellation point in the calling thread.
 *
 * This function is part of the POSIX standard and is used to create a cancellation point in the calling thread,
 * allowing it to be canceled if a cancellation request is pending. However, in this OS environment, this function is not supported
 * and does not perform any action.
 */
extern void pthread_testcancel(void);

/*
Note: The following functions are not supported and are not defined in this environment.
- pthread_attr_setinheritsched
- pthread_attr_getinheritsched
- pthread_attr_setaffinity_np
- pthread_attr_getaffinity_np
- pthread_setaffinity_np
- pthread_getaffinity_np
- pthread_setschedprio
- pthread_setcancelstate(int state, int *oldstate);
*/

#ifdef  __cplusplus
}
#endif
