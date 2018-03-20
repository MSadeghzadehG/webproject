

package com.google.common.util.concurrent;

import com.google.caliper.BeforeExperiment;
import com.google.caliper.Benchmark;
import com.google.caliper.Param;
import java.lang.reflect.Constructor;
import java.util.concurrent.BlockingQueue;


public class MonitorBenchmark {

  @Param({"10", "100", "1000"})
  int capacity;

  @Param({"Array", "Priority"})
  String queueType;

  @Param boolean useMonitor;

  private BlockingQueue<String> queue;
  private String[] strings;

  @BeforeExperiment
  @SuppressWarnings("unchecked")
  void setUp() throws Exception {
    String prefix =
        (useMonitor ? "com.google.common.util.concurrent.MonitorBased" : "java.util.concurrent.");
    String className = prefix + queueType + "BlockingQueue";
    Constructor<?> constructor = Class.forName(className).getConstructor(int.class);
    queue = (BlockingQueue<String>) constructor.newInstance(capacity);

    strings = new String[capacity];
    for (int i = 0; i < capacity; i++) {
      strings[i] = String.valueOf(Math.random());
    }
  }

  @Benchmark
  void addsAndRemoves(int reps) {
    int capacity = this.capacity;
    BlockingQueue<String> queue = this.queue;
    String[] strings = this.strings;
    for (int i = 0; i < reps; i++) {
      for (int j = 0; j < capacity; j++) {
        queue.add(strings[j]);
      }
      for (int j = 0; j < capacity; j++) {
        queue.remove();
      }
    }
  }
}
