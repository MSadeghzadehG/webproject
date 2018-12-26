

package com.google.common.collect;

import com.google.caliper.BeforeExperiment;
import com.google.caliper.Benchmark;
import com.google.caliper.Param;
import java.util.Arrays;
import java.util.Comparator;
import java.util.Random;


public class ComparatorDelegationOverheadBenchmark {
  private final Integer[][] inputArrays = new Integer[0x100][];

  @Param({"10000"})
  int n;

  @BeforeExperiment
  void setUp() throws Exception {
    Random rng = new Random();
    for (int i = 0; i < 0x100; i++) {
      Integer[] array = new Integer[n];
      for (int j = 0; j < n; j++) {
        array[j] = rng.nextInt();
      }
      inputArrays[i] = array;
    }
  }

  @Benchmark
  int arraysSortNoComparator(int reps) {
    int tmp = 0;
    for (int i = 0; i < reps; i++) {
      Integer[] copy = inputArrays[i & 0xFF].clone();
      Arrays.sort(copy);
      tmp += copy[0];
    }
    return tmp;
  }

  @Benchmark
  int arraysSortOrderingNatural(int reps) {
    int tmp = 0;
    for (int i = 0; i < reps; i++) {
      Integer[] copy = inputArrays[i & 0xFF].clone();
      Arrays.sort(copy, Ordering.natural());
      tmp += copy[0];
    }
    return tmp;
  }

  private static final Comparator<Integer> NATURAL_INTEGER =
      new Comparator<Integer>() {
        @Override
        public int compare(Integer o1, Integer o2) {
          return o1.compareTo(o2);
        }
      };

  @Benchmark
  int arraysSortOrderingFromNatural(int reps) {
    int tmp = 0;
    for (int i = 0; i < reps; i++) {
      Integer[] copy = inputArrays[i & 0xFF].clone();
      Arrays.sort(copy, Ordering.from(NATURAL_INTEGER));
      tmp += copy[0];
    }
    return tmp;
  }
}
