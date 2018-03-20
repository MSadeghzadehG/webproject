

package com.google.common.util.concurrent;

import static com.google.common.util.concurrent.MoreExecutors.directExecutor;

import com.google.caliper.BeforeExperiment;
import com.google.caliper.Param;
import com.google.caliper.api.Footprint;
import com.google.caliper.api.SkipThisScenarioException;
import com.google.common.util.concurrent.AbstractFutureBenchmarks.Facade;
import com.google.common.util.concurrent.AbstractFutureBenchmarks.Impl;
import java.util.HashSet;
import java.util.Set;
import java.util.concurrent.Executor;


public class AbstractFutureFootprintBenchmark {

  enum State {
    NOT_DONE,
    FINISHED,
    CANCELLED,
    FAILED
  }

  @Param State state;
  @Param Impl impl;

  @Param({"0", "1", "5", "10"})
  int numListeners;

  @Param({"0", "1", "5", "10"})
  int numThreads;

  private final Set<Thread> blockedThreads = new HashSet<>();

  @BeforeExperiment
  void setUp() throws Exception {
    if (state != State.NOT_DONE && (numListeners != 0 || numThreads != 0)) {
      throw new SkipThisScenarioException();
    }
  }

      @SuppressWarnings("FutureReturnValueIgnored")
  @Footprint(exclude = {Runnable.class, Executor.class, Thread.class, Exception.class})
  public Object measureSize() {
    for (Thread thread : blockedThreads) {
      thread.interrupt();
    }
    blockedThreads.clear();
    final Facade<Object> f = impl.newFacade();
    for (int i = 0; i < numThreads; i++) {
      Thread thread =
          new Thread() {
            @Override
            public void run() {
              try {
                f.get();
              } catch (Throwable expected) {
              }
            }
          };
      thread.start();
      blockedThreads.add(thread);
    }
    for (int i = 0; i < numListeners; i++) {
      f.addListener(Runnables.doNothing(), directExecutor());
    }
    for (Thread thread : blockedThreads) {
      AbstractFutureBenchmarks.awaitWaiting(thread);
    }
    switch (state) {
      case NOT_DONE:
        break;
      case FINISHED:
        f.set(null);
        break;
      case CANCELLED:
        f.cancel(false);
        break;
      case FAILED:
        f.setException(new Exception());
        break;
      default:
        throw new AssertionError();
    }
    return f;
  }
}
