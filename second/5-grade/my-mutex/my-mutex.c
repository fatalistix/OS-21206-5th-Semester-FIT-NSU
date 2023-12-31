/*
 * Copyright (c) 2023, Vyaheslav Balashov
 */

#include "./my-mutex.h"

#include <atomic_ops.h>
#include <errno.h>
#include <linux/futex.h>
#include <stdatomic.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <unistd.h>

static long futex(uint32_t *uaddr, int futex_op, uint32_t val,
                  const struct timespec *timeout, uint32_t *uaddr2,
                  uint32_t val3) {
  return syscall(SYS_futex, uaddr, futex_op, val, timeout, uaddr2, val3);
}

static long futex_wait(uint32_t *futx, uint32_t wait_value) {
  return futex(futx, FUTEX_WAIT, wait_value, NULL, NULL, 0);
}

static long futex_wake(uint32_t *futx, uint32_t num_to_wake) {
  return futex(futx, FUTEX_WAKE, num_to_wake, NULL, NULL, 0);
}

enum { UNLOCKED, LOCKED_NO_WAITERS, LOCKED_WITH_WAITERS };

int my_mutex_init(my_mutex_t *m) {
  *m = UNLOCKED;
  return 0;
}

int my_mutex_lock(my_mutex_t *m) {
  int c;
  long err;
  if ((c = __sync_val_compare_and_swap(m, UNLOCKED, LOCKED_NO_WAITERS) !=
           UNLOCKED)) {
    if (c == LOCKED_NO_WAITERS) {
      c = atomic_exchange(m, LOCKED_WITH_WAITERS);
    }
    while (c != UNLOCKED) {
      err = futex_wait(m, LOCKED_WITH_WAITERS);
      if (err == -1 && errno != EAGAIN) {
        return err;
      }
      c = atomic_exchange(m, LOCKED_WITH_WAITERS);
    }
  }
  return 0;
}

int my_mutex_unlock(my_mutex_t *m) {
  if (atomic_fetch_sub(m, 1) != LOCKED_NO_WAITERS) {
    atomic_exchange(m, UNLOCKED);
    futex_wake(m, 1);
  }
  return 0;
}
