/*
 * spinlock.c
 * Definitions of the functions that handle
 * the spinlock mechanism
 */
#include "spinlock.h"

/*
 * spin_lock
 *   DESCRIPTION: This function is used to acquire a lock
 *                It is used to protect critical sections
 *   INPUTS: lock - Pointer to spinlock_t
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Lock is acquired when free
 */
void
spin_lock(spinlock_t* lock)
{
  asm volatile(
     "spin_wait:          \n\t"
     "xchgl (%1), %%eax   \n\t"
     "cmpl $1, %%eax      \n\t"
     "je spin_wait        \n\t"
     "mov $1, %%eax       \n\t"
     "xchgl %%eax, (%0)   \n\t"
     "ret"
     : "=r"(lock)
     : "r"(lock)
     : "memory"
  );
}

/*
 * spin_unlock
 *   DESCRIPTION: This is used to release control of the lock
 *   INPUTS: lock - Pointer to spinlock_t
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Lock is released or made available
 */
void
spin_unlock(spinlock_t* lock)
{
  asm volatile(
    "releaseLock:           \n\t"
    "mov $0, %%eax          \n\t"
    "xchgl %%eax, (%0)      \n\t"
    "ret"
    : "=r" (lock)
    :
    : "cc", "memory"
  );
}


/*
 * spin_lock_irqsave
 *   DESCRIPTION: This function is used to acquire a lock
 *                It is used to protect critical sections
 *   INPUTS: lock - Pointer to spinlock_t
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Lock is acquired when free
 */
void
spin_lock_irqsave(spinlock_t* lock, unsigned long flag)
{
  asm volatile(
    "pushfl                 \n\t"
    "mov (%%esp), %%eax     \n\t"
    "xchgl %%eax, (%0)      \n\t"
    "cli"
    : "=r" (flag)
    :
    : "cc", "memory"
  );
  spin_lock(lock);
}

/*
 * spin_unlock_irqrestore
 *   DESCRIPTION: This function is used to acquire a lock
 *                It is used to protect critical sections
 *   INPUTS: lock - Pointer to spinlock_t
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Lock is acquired when free
 */
void
spin_unlock_irqrestore(spinlock_t* lock, unsigned long flag)
{
  asm volatile(
    "mov $0, %%eax           \n\t"
    "xchgl %%eax, (%0)       \n\t"
    "pushl %1                \n\t"
    "popfl"
    : "=r"(lock)
    : "r" (flag)
    : "cc", "memory"
  );
}
