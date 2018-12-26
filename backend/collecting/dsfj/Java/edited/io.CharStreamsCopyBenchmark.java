

package com.google.common.io;

import com.google.caliper.BeforeExperiment;
import com.google.caliper.Benchmark;
import com.google.caliper.Param;
import com.google.caliper.api.VmOptions;
import java.io.IOException;
import java.io.StringReader;
import java.io.StringWriter;
import java.nio.CharBuffer;
import java.util.Random;


@VmOptions({"-Xms12g", "-Xmx12g", "-d64"})
public class CharStreamsCopyBenchmark {
  enum CopyStrategy {
    OLD {
      @Override
      long copy(Readable from, Appendable to) throws IOException {
        CharBuffer buf = CharStreams.createBuffer();
        long total = 0;
        while (from.read(buf) != -1) {
          buf.flip();
          to.append(buf);
          total += buf.remaining();
          buf.clear();
        }
        return total;
      }
    },
    NEW {
      @Override
      long copy(Readable from, Appendable to) throws IOException {
        return CharStreams.copy(from, to);
      }
    };

    abstract long copy(Readable from, Appendable to) throws IOException;
  }

  enum TargetSupplier {
    STRING_WRITER {
      @Override
      Appendable get(int sz) {
        return new StringWriter(sz);
      }
    },
    STRING_BUILDER {
      @Override
      Appendable get(int sz) {
        return new StringBuilder(sz);
      }
    };

    abstract Appendable get(int sz);
  }

  @Param CopyStrategy strategy;
  @Param TargetSupplier target;

  @Param({"10", "1024", "1048576"})
  int size;

  String data;

  @BeforeExperiment
  public void setUp() {
        StringBuilder sb = new StringBuilder();
    Random random = new Random(0xdeadbeef);     sb.ensureCapacity(size);
    for (int k = 0; k < size; k++) {
            sb.append((char) (random.nextInt(127 - 9) + 9));
    }
    data = sb.toString();
  }

  @Benchmark
  public long timeCopy(int reps) throws IOException {
    long r = 0;
    final String localData = data;
    final TargetSupplier localTarget = target;
    final CopyStrategy localStrategy = strategy;
    for (int i = 0; i < reps; i++) {
      Appendable appendable = localTarget.get(localData.length());
      r += localStrategy.copy(new StringReader(localData), appendable);
    }
    return r;
  }
}
