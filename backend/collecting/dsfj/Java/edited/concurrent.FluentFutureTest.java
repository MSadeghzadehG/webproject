

package com.google.common.util.concurrent;

import static com.google.common.base.Verify.verify;
import static com.google.common.truth.Truth.assertThat;
import static com.google.common.util.concurrent.Futures.immediateFailedFuture;
import static com.google.common.util.concurrent.Futures.immediateFuture;
import static com.google.common.util.concurrent.MoreExecutors.directExecutor;
import static java.util.concurrent.Executors.newScheduledThreadPool;
import static java.util.concurrent.TimeUnit.SECONDS;

import com.google.common.annotations.GwtCompatible;
import com.google.common.annotations.GwtIncompatible;
import com.google.common.base.Function;
import com.google.common.util.concurrent.ForwardingListenableFuture.SimpleForwardingListenableFuture;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeoutException;
import junit.framework.TestCase;


@GwtCompatible(emulated = true)
public class FluentFutureTest extends TestCase {
  public void testFromFluentFuture() {
    FluentFuture<String> f = SettableFuture.create();
    assertThat(FluentFuture.from(f)).isSameAs(f);
  }

  public void testFromNonFluentFuture() throws Exception {
    ListenableFuture<String> f =
        new SimpleForwardingListenableFuture<String>(immediateFuture("a")) {};
    verify(!(f instanceof FluentFuture));
    assertThat(FluentFuture.from(f).get()).isEqualTo("a");
      }

  public void testAddCallback() {
    FluentFuture<String> f = FluentFuture.from(immediateFuture("a"));
    final boolean[] called = new boolean[1];
    f.addCallback(
        new FutureCallback<String>() {
          @Override
          public void onSuccess(String result) {
            called[0] = true;
          }

          @Override
          public void onFailure(Throwable t) {}
        },
        directExecutor());
    assertThat(called[0]).isTrue();
  }

  public void testCatching() throws Exception {
    FluentFuture<?> f =
        FluentFuture.from(immediateFailedFuture(new RuntimeException()))
            .catching(
                Throwable.class,
                new Function<Throwable, Class<?>>() {
                  @Override
                  public Class<?> apply(Throwable input) {
                    return input.getClass();
                  }
                },
                directExecutor());
    assertThat(f.get()).isEqualTo(RuntimeException.class);
  }

  public void testCatchingAsync() throws Exception {
    FluentFuture<?> f =
        FluentFuture.from(immediateFailedFuture(new RuntimeException()))
            .catchingAsync(
                Throwable.class,
                new AsyncFunction<Throwable, Class<?>>() {
                  @Override
                  public ListenableFuture<Class<?>> apply(Throwable input) {
                    return Futures.<Class<?>>immediateFuture(input.getClass());
                  }
                },
                directExecutor());
    assertThat(f.get()).isEqualTo(RuntimeException.class);
  }

  public void testTransform() throws Exception {
    FluentFuture<Integer> f =
        FluentFuture.from(immediateFuture(1))
            .transform(
                new Function<Integer, Integer>() {
                  @Override
                  public Integer apply(Integer input) {
                    return input + 1;
                  }
                },
                directExecutor());
    assertThat(f.get()).isEqualTo(2);
  }

  public void testTransformAsync() throws Exception {
    FluentFuture<Integer> f =
        FluentFuture.from(immediateFuture(1))
            .transformAsync(
                new AsyncFunction<Integer, Integer>() {
                  @Override
                  public ListenableFuture<Integer> apply(Integer input) {
                    return immediateFuture(input + 1);
                  }
                },
                directExecutor());
    assertThat(f.get()).isEqualTo(2);
  }

  @GwtIncompatible   public void testWithTimeout() throws Exception {
    ScheduledExecutorService executor = newScheduledThreadPool(1);
    try {
      FluentFuture<?> f = SettableFuture.create().withTimeout(0, SECONDS, executor);
      try {
        f.get();
        fail();
      } catch (ExecutionException e) {
        assertThat(e.getCause()).isInstanceOf(TimeoutException.class);
      }
    } finally {
      executor.shutdown();
    }
  }
}
