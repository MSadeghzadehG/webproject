

package com.google.common.testing;

import static java.util.concurrent.TimeUnit.SECONDS;

import com.google.common.annotations.Beta;
import com.google.common.annotations.GwtIncompatible;
import com.google.j2objc.annotations.J2ObjCIncompatible;
import java.lang.ref.WeakReference;
import java.util.Locale;
import java.util.concurrent.CancellationException;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Future;
import java.util.concurrent.TimeoutException;


@Beta
@GwtIncompatible
@J2ObjCIncompatible public final class GcFinalization {
  private GcFinalization() {}

  
  private static long timeoutSeconds() {
                                                return Math.max(10L, Runtime.getRuntime().totalMemory() / (32L * 1024L * 1024L));
  }

  
  public static void awaitDone(Future<?> future) {
    if (future.isDone()) {
      return;
    }
    final long timeoutSeconds = timeoutSeconds();
    final long deadline = System.nanoTime() + SECONDS.toNanos(timeoutSeconds);
    do {
      System.runFinalization();
      if (future.isDone()) {
        return;
      }
      System.gc();
      try {
        future.get(1L, SECONDS);
        return;
      } catch (CancellationException | ExecutionException ok) {
        return;
      } catch (InterruptedException ie) {
        throw new RuntimeException("Unexpected interrupt while waiting for future", ie);
      } catch (TimeoutException tryHarder) {
        
      }
    } while (System.nanoTime() - deadline < 0);
    throw formatRuntimeException("Future not done within %d second timeout", timeoutSeconds);
  }

  
  public static void awaitDone(FinalizationPredicate predicate) {
    if (predicate.isDone()) {
      return;
    }
    final long timeoutSeconds = timeoutSeconds();
    final long deadline = System.nanoTime() + SECONDS.toNanos(timeoutSeconds);
    do {
      System.runFinalization();
      if (predicate.isDone()) {
        return;
      }
      CountDownLatch done = new CountDownLatch(1);
      createUnreachableLatchFinalizer(done);
      await(done);
      if (predicate.isDone()) {
        return;
      }
    } while (System.nanoTime() - deadline < 0);
    throw formatRuntimeException(
        "Predicate did not become true within %d second timeout", timeoutSeconds);
  }

  
  public static void await(CountDownLatch latch) {
    if (latch.getCount() == 0) {
      return;
    }
    final long timeoutSeconds = timeoutSeconds();
    final long deadline = System.nanoTime() + SECONDS.toNanos(timeoutSeconds);
    do {
      System.runFinalization();
      if (latch.getCount() == 0) {
        return;
      }
      System.gc();
      try {
        if (latch.await(1L, SECONDS)) {
          return;
        }
      } catch (InterruptedException ie) {
        throw new RuntimeException("Unexpected interrupt while waiting for latch", ie);
      }
    } while (System.nanoTime() - deadline < 0);
    throw formatRuntimeException(
        "Latch failed to count down within %d second timeout", timeoutSeconds);
  }

  
  private static void createUnreachableLatchFinalizer(final CountDownLatch latch) {
    new Object() {
      @Override
      protected void finalize() {
        latch.countDown();
      }
    };
  }

  
  public interface FinalizationPredicate {
    boolean isDone();
  }

  
  public static void awaitClear(final WeakReference<?> ref) {
    awaitDone(
        new FinalizationPredicate() {
          public boolean isDone() {
            return ref.get() == null;
          }
        });
  }

  
  public static void awaitFullGc() {
    final CountDownLatch finalizerRan = new CountDownLatch(1);
    WeakReference<Object> ref =
        new WeakReference<Object>(
            new Object() {
              @Override
              protected void finalize() {
                finalizerRan.countDown();
              }
            });

    await(finalizerRan);
    awaitClear(ref);

        System.runFinalization();
  }

  private static RuntimeException formatRuntimeException(String format, Object... args) {
    return new RuntimeException(String.format(Locale.ROOT, format, args));
  }
}
