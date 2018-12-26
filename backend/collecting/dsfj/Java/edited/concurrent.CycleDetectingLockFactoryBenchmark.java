

package com.google.common.util.concurrent;

import com.google.caliper.BeforeExperiment;
import com.google.caliper.Benchmark;
import com.google.caliper.Param;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;


public class CycleDetectingLockFactoryBenchmark {

  @Param({"2", "3", "4", "5", "10"})
  int lockNestingDepth;

  CycleDetectingLockFactory factory;
  private Lock[] plainLocks;
  private Lock[] detectingLocks;

  @BeforeExperiment
  void setUp() throws Exception {
    this.factory = CycleDetectingLockFactory.newInstance(CycleDetectingLockFactory.Policies.WARN);
    this.plainLocks = new Lock[lockNestingDepth];
    for (int i = 0; i < lockNestingDepth; i++) {
      plainLocks[i] = new ReentrantLock();
    }
    this.detectingLocks = new Lock[lockNestingDepth];
    for (int i = 0; i < lockNestingDepth; i++) {
      detectingLocks[i] = factory.newReentrantLock("Lock" + i);
    }
  }

  @Benchmark
  void unorderedPlainLocks(int reps) {
    lockAndUnlock(new ReentrantLock(), reps);
  }

  @Benchmark
  void unorderedCycleDetectingLocks(int reps) {
    lockAndUnlock(factory.newReentrantLock("foo"), reps);
  }

  private static void lockAndUnlock(Lock lock, int reps) {
    for (int i = 0; i < reps; i++) {
      lock.lock();
      lock.unlock();
    }
  }

  @Benchmark
  void orderedPlainLocks(int reps) {
    lockAndUnlockNested(plainLocks, reps);
  }

  @Benchmark
  void orderedCycleDetectingLocks(int reps) {
    lockAndUnlockNested(detectingLocks, reps);
  }

  private static void lockAndUnlockNested(Lock[] locks, int reps) {
    for (int i = 0; i < reps; i++) {
      for (int j = 0; j < locks.length; j++) {
        locks[j].lock();
      }
      for (int j = locks.length - 1; j >= 0; j--) {
        locks[j].unlock();
      }
    }
  }
}
