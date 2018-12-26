

package com.google.common.collect;

import com.google.caliper.BeforeExperiment;
import com.google.caliper.Benchmark;
import java.util.ArrayList;
import java.util.List;
import java.util.Random;


public class HashMultisetAddPresentBenchmark {
  private static final int ARRAY_MASK = 0x0ffff;
  private static final int ARRAY_SIZE = 0x10000;
  List<Multiset<Integer>> multisets = new ArrayList<>(0x10000);
  int[] queries = new int[ARRAY_SIZE];

  @BeforeExperiment
  void setUp() {
    Random random = new Random();
    multisets.clear();
    for (int i = 0; i < ARRAY_SIZE; i++) {
      HashMultiset<Integer> multiset = HashMultiset.<Integer>create();
      multisets.add(multiset);
      queries[i] = random.nextInt();
      multiset.add(queries[i]);
    }
  }

  @Benchmark
  int add(int reps) {
    int tmp = 0;
    for (int i = 0; i < reps; i++) {
      int j = i & ARRAY_MASK;
      tmp += multisets.get(j).add(queries[j], 4);
    }
    return tmp;
  }
}
