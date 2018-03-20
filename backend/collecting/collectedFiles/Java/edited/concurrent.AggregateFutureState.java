

package com.google.common.util.concurrent;

import static com.google.common.collect.Sets.newConcurrentHashSet;
import static java.util.concurrent.atomic.AtomicIntegerFieldUpdater.newUpdater;
import static java.util.concurrent.atomic.AtomicReferenceFieldUpdater.newUpdater;

import com.google.common.annotations.GwtCompatible;
import com.google.j2objc.annotations.ReflectionSupport;
import java.util.Set;
import java.util.concurrent.atomic.AtomicIntegerFieldUpdater;
import java.util.concurrent.atomic.AtomicReferenceFieldUpdater;
import java.util.logging.Level;
import java.util.logging.Logger;


@GwtCompatible(emulated = true)
@ReflectionSupport(value = ReflectionSupport.Level.FULL)
abstract class AggregateFutureState {
      private volatile Set<Throwable> seenExceptions = null;

  private volatile int remaining;

  private static final AtomicHelper ATOMIC_HELPER;

  private static final Logger log = Logger.getLogger(AggregateFutureState.class.getName());

  static {
    AtomicHelper helper;
    Throwable thrownReflectionFailure = null;
    try {
      helper =
          new SafeAtomicHelper(
              newUpdater(AggregateFutureState.class, (Class) Set.class, "seenExceptions"),
              newUpdater(AggregateFutureState.class, "remaining"));
    } catch (Throwable reflectionFailure) {
                              thrownReflectionFailure = reflectionFailure;
      helper = new SynchronizedAtomicHelper();
    }
    ATOMIC_HELPER = helper;
            if (thrownReflectionFailure != null) {
      log.log(Level.SEVERE, "SafeAtomicHelper is broken!", thrownReflectionFailure);
    }
  }

  AggregateFutureState(int remainingFutures) {
    this.remaining = remainingFutures;
  }

  final Set<Throwable> getOrInitSeenExceptions() {
    
    Set<Throwable> seenExceptionsLocal = seenExceptions;
    if (seenExceptionsLocal == null) {
      seenExceptionsLocal = newConcurrentHashSet();
      
      addInitialException(seenExceptionsLocal);

      ATOMIC_HELPER.compareAndSetSeenExceptions(this, null, seenExceptionsLocal);
      
      seenExceptionsLocal = seenExceptions;
    }
    return seenExceptionsLocal;
  }

  
  abstract void addInitialException(Set<Throwable> seen);

  final int decrementRemainingAndGet() {
    return ATOMIC_HELPER.decrementAndGetRemainingCount(this);
  }

  private abstract static class AtomicHelper {
    
    abstract void compareAndSetSeenExceptions(
        AggregateFutureState state, Set<Throwable> expect, Set<Throwable> update);

    
    abstract int decrementAndGetRemainingCount(AggregateFutureState state);
  }

  private static final class SafeAtomicHelper extends AtomicHelper {
    final AtomicReferenceFieldUpdater<AggregateFutureState, Set<Throwable>> seenExceptionsUpdater;

    final AtomicIntegerFieldUpdater<AggregateFutureState> remainingCountUpdater;

    SafeAtomicHelper(
        AtomicReferenceFieldUpdater seenExceptionsUpdater,
        AtomicIntegerFieldUpdater remainingCountUpdater) {
      this.seenExceptionsUpdater = seenExceptionsUpdater;
      this.remainingCountUpdater = remainingCountUpdater;
    }

    @Override
    void compareAndSetSeenExceptions(
        AggregateFutureState state, Set<Throwable> expect, Set<Throwable> update) {
      seenExceptionsUpdater.compareAndSet(state, expect, update);
    }

    @Override
    int decrementAndGetRemainingCount(AggregateFutureState state) {
      return remainingCountUpdater.decrementAndGet(state);
    }
  }

  private static final class SynchronizedAtomicHelper extends AtomicHelper {
    @Override
    void compareAndSetSeenExceptions(
        AggregateFutureState state, Set<Throwable> expect, Set<Throwable> update) {
      synchronized (state) {
        if (state.seenExceptions == expect) {
          state.seenExceptions = update;
        }
      }
    }

    @Override
    int decrementAndGetRemainingCount(AggregateFutureState state) {
      synchronized (state) {
        state.remaining--;
        return state.remaining;
      }
    }
  }
}
