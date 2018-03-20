

package com.google.common.util.concurrent;

import com.google.caliper.BeforeExperiment;
import com.google.caliper.Benchmark;
import com.google.caliper.Param;
import com.google.caliper.api.VmOptions;
import com.google.common.util.concurrent.AbstractFutureBenchmarks.Facade;
import com.google.common.util.concurrent.AbstractFutureBenchmarks.Impl;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CancellationException;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;


@VmOptions({"-Xms8g", "-Xmx8g"})
public class SingleThreadAbstractFutureBenchmark {
  @Param Impl impl;

  private final Exception exception = new Exception();
  private Facade<?> notDoneFuture;

  @BeforeExperiment
  void setUp() throws Exception {
    notDoneFuture = impl.newFacade();
  }

  @Benchmark
  public long timeComplete_Normal(int reps) throws Exception {
    long r = 0;
    List<Facade<Integer>> list = new ArrayList<>(reps);
    for (int i = 0; i < reps; i++) {
      final Facade<Integer> localFuture = impl.newFacade();
      list.add(localFuture);
      localFuture.set(i);
    }
    for (int i = 0; i < reps; i++) {
      r += list.get(i).get();
    }
    return r;
  }

  @Benchmark
  public long timeComplete_Failure(int reps) throws Exception {
    long r = 0;
    List<Facade<Integer>> list = new ArrayList<>(reps);
    for (int i = 0; i < reps; i++) {
      final Facade<Integer> localFuture = impl.newFacade();
      list.add(localFuture);
      localFuture.setException(exception);
    }
    for (int i = 0; i < reps; i++) {
      Facade<Integer> facade = list.get(i);
      try {
        facade.get();
        r++;
      } catch (ExecutionException e) {
        r += 2;
      }
    }
    return r;
  }

  @Benchmark
  public long timeComplete_Cancel(int reps) throws Exception {
    long r = 0;
    List<Facade<Integer>> list = new ArrayList<>(reps);
    for (int i = 0; i < reps; i++) {
      final Facade<Integer> localFuture = impl.newFacade();
      list.add(localFuture);
      localFuture.cancel(false);
    }
    for (int i = 0; i < reps; i++) {
      Facade<Integer> facade = list.get(i);
      try {
        facade.get();
        r++;
      } catch (CancellationException e) {
        r += 2;
      }
    }
    return r;
  }

  @Benchmark
  public long timeGetWith0Timeout(long reps) throws Exception {
    Facade<?> f = notDoneFuture;
    long r = 0;
    for (int i = 0; i < reps; i++) {
      try {
        f.get(0, TimeUnit.SECONDS);
        r += 1;
      } catch (TimeoutException e) {
        r += 2;
      }
    }
    return r;
  }

  @Benchmark
  public long timeGetWithSmallTimeout(long reps) throws Exception {
    Facade<?> f = notDoneFuture;
    long r = 0;
    for (int i = 0; i < reps; i++) {
      try {
        f.get(500, TimeUnit.NANOSECONDS);
        r += 1;
      } catch (TimeoutException e) {
        r += 2;
      }
    }
    return r;
  }
}
