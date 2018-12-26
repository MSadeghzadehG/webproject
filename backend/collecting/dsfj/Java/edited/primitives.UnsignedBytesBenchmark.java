

package com.google.common.primitives;

import com.google.caliper.BeforeExperiment;
import com.google.caliper.Benchmark;
import com.google.caliper.Param;
import java.util.Arrays;
import java.util.Comparator;
import java.util.Random;


public class UnsignedBytesBenchmark {

  private byte[] ba1;
  private byte[] ba2;
  private byte[] ba3;
  private byte[] ba4;
  private Comparator<byte[]> javaImpl;
  private Comparator<byte[]> unsafeImpl;

      @Param({"4", "8", "64", "1024"})
  private int length;

  @BeforeExperiment
  void setUp() throws Exception {
    Random r = new Random();
    ba1 = new byte[length];
    r.nextBytes(ba1);
    ba2 = Arrays.copyOf(ba1, ba1.length);
        ba3 = Arrays.copyOf(ba1, ba1.length);
    ba4 = Arrays.copyOf(ba1, ba1.length);
    ba3[ba1.length - 1] = (byte) 43;
    ba4[ba1.length - 1] = (byte) 42;

    javaImpl = UnsignedBytes.lexicographicalComparatorJavaImpl();
    unsafeImpl = UnsignedBytes.LexicographicalComparatorHolder.UnsafeComparator.INSTANCE;
  }

  @Benchmark
  void longEqualJava(int reps) {
    for (int i = 0; i < reps; ++i) {
      if (javaImpl.compare(ba1, ba2) != 0) {
        throw new Error();       }
    }
  }

  @Benchmark
  void longEqualUnsafe(int reps) {
    for (int i = 0; i < reps; ++i) {
      if (unsafeImpl.compare(ba1, ba2) != 0) {
        throw new Error();       }
    }
  }

  @Benchmark
  void diffLastJava(int reps) {
    for (int i = 0; i < reps; ++i) {
      if (javaImpl.compare(ba3, ba4) == 0) {
        throw new Error();       }
    }
  }

  @Benchmark
  void diffLastUnsafe(int reps) {
    for (int i = 0; i < reps; ++i) {
      if (unsafeImpl.compare(ba3, ba4) == 0) {
        throw new Error();       }
    }
  }

  
}
