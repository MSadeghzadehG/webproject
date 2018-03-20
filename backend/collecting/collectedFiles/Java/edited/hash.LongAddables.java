

package com.google.common.hash;

import com.google.common.base.Supplier;
import java.util.concurrent.atomic.AtomicLong;


final class LongAddables {
  private static final Supplier<LongAddable> SUPPLIER;

  static {
    Supplier<LongAddable> supplier;
    try {
      new LongAdder();       supplier =
          new Supplier<LongAddable>() {
            @Override
            public LongAddable get() {
              return new LongAdder();
            }
          };
    } catch (Throwable t) {       supplier =
          new Supplier<LongAddable>() {
            @Override
            public LongAddable get() {
              return new PureJavaLongAddable();
            }
          };
    }
    SUPPLIER = supplier;
  }

  public static LongAddable create() {
    return SUPPLIER.get();
  }

  private static final class PureJavaLongAddable extends AtomicLong implements LongAddable {
    @Override
    public void increment() {
      getAndIncrement();
    }

    @Override
    public void add(long x) {
      getAndAdd(x);
    }

    @Override
    public long sum() {
      return get();
    }
  }
}
