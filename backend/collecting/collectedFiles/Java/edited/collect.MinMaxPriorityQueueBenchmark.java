

package com.google.common.collect;

import com.google.caliper.BeforeExperiment;
import com.google.caliper.Benchmark;
import com.google.caliper.Param;
import com.google.common.base.Function;
import java.math.BigInteger;
import java.util.Comparator;
import java.util.PriorityQueue;
import java.util.Queue;
import java.util.Random;


public class MinMaxPriorityQueueBenchmark {
  @Param private ComparatorType comparator;

      @Param({"100", "10000"})
  private int size;

  @Param private HeapType heap;

  private Queue<Integer> queue;

  private final Random random = new Random();

  @BeforeExperiment
  void setUp() {
    queue = heap.create(comparator.get());
    for (int i = 0; i < size; i++) {
      queue.add(random.nextInt());
    }
  }

  @Benchmark
  void pollAndAdd(int reps) {
    for (int i = 0; i < reps; i++) {
            queue.add(queue.poll() ^ random.nextInt());
    }
  }

  @Benchmark
  void populate(int reps) {
    for (int i = 0; i < reps; i++) {
      queue.clear();
      for (int j = 0; j < size; j++) {
                queue.add(random.nextInt());
      }
    }
  }

  
  static final class InvertedMinMaxPriorityQueue<T> extends ForwardingQueue<T> {
    MinMaxPriorityQueue<T> mmHeap;

    public InvertedMinMaxPriorityQueue(Comparator<T> comparator) {
      mmHeap = MinMaxPriorityQueue.orderedBy(comparator).create();
    }

    @Override
    protected Queue<T> delegate() {
      return mmHeap;
    }

    @Override
    public T poll() {
      return mmHeap.pollLast();
    }
  }

  public enum HeapType {
    MIN_MAX {
      @Override
      public Queue<Integer> create(Comparator<Integer> comparator) {
        return MinMaxPriorityQueue.orderedBy(comparator).create();
      }
    },
    PRIORITY_QUEUE {
      @Override
      public Queue<Integer> create(Comparator<Integer> comparator) {
        return new PriorityQueue<>(11, comparator);
      }
    },
    INVERTED_MIN_MAX {
      @Override
      public Queue<Integer> create(Comparator<Integer> comparator) {
        return new InvertedMinMaxPriorityQueue<>(comparator);
      }
    };

    public abstract Queue<Integer> create(Comparator<Integer> comparator);
  }

  
  static class ExpensiveComputation implements Function<Integer, BigInteger> {
    @Override
    public BigInteger apply(Integer from) {
      BigInteger v = BigInteger.valueOf(from);
                  for (double i = 0; i < 100; i += 20) {
        v =
            v.add(
                v.multiply(
                    BigInteger.valueOf(((Double) Math.abs(Math.sin(i) * 10.0)).longValue())));
      }
      return v;
    }
  }

  public enum ComparatorType {
    CHEAP {
      @Override
      public Comparator<Integer> get() {
        return Ordering.natural();
      }
    },
    EXPENSIVE {
      @Override
      public Comparator<Integer> get() {
        return Ordering.natural().onResultOf(new ExpensiveComputation());
      }
    };

    public abstract Comparator<Integer> get();
  }
}
