/*
 * spinlock.h
 * Declares the handling functions for the spinlocks
 */

#ifndef SPINLOCK_H
#define SPINLOCK_H

#include "../types.h"

#define SPIN_LOCK_UNLOCKED        0

typedef struct {
  volatile unsigned long lock;
} spinlock_t;

/* Functions for implementing the locking mechanism */
void spin_lock(spinlock_t* lock);

void spin_unlock(spinlock_t* lock);

void spin_lock_irqsave(spinlock_t* lock, unsigned long flag);

void spin_unlock_irqrestore(spinlock_t* lock, unsigned long flag);

#endif
