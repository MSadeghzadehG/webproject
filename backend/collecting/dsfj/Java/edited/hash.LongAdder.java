



package com.google.common.hash;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.Serializable;
import java.util.concurrent.atomic.AtomicLong;


final class LongAdder extends Striped64 implements Serializable, LongAddable {
  private static final long serialVersionUID = 7249069246863182397L;

  
  final long fn(long v, long x) {
    return v + x;
  }

  
  public LongAdder() {}

  
  public void add(long x) {
    Cell[] as;
    long b, v;
    int[] hc;
    Cell a;
    int n;
    if ((as = cells) != null || !casBase(b = base, b + x)) {
      boolean uncontended = true;
      if ((hc = threadHashCode.get()) == null
          || as == null
          || (n = as.length) < 1
          || (a = as[(n - 1) & hc[0]]) == null
          || !(uncontended = a.cas(v = a.value, v + x))) retryUpdate(x, hc, uncontended);
    }
  }

  
  public void increment() {
    add(1L);
  }

  
  public void decrement() {
    add(-1L);
  }

  
  public long sum() {
    long sum = base;
    Cell[] as = cells;
    if (as != null) {
      int n = as.length;
      for (int i = 0; i < n; ++i) {
        Cell a = as[i];
        if (a != null) sum += a.value;
      }
    }
    return sum;
  }

  
  public void reset() {
    internalReset(0L);
  }

  
  public long sumThenReset() {
    long sum = base;
    Cell[] as = cells;
    base = 0L;
    if (as != null) {
      int n = as.length;
      for (int i = 0; i < n; ++i) {
        Cell a = as[i];
        if (a != null) {
          sum += a.value;
          a.value = 0L;
        }
      }
    }
    return sum;
  }

  
  public String toString() {
    return Long.toString(sum());
  }

  
  public long longValue() {
    return sum();
  }

  
  public int intValue() {
    return (int) sum();
  }

  
  public float floatValue() {
    return (float) sum();
  }

  
  public double doubleValue() {
    return (double) sum();
  }

  private void writeObject(ObjectOutputStream s) throws IOException {
    s.defaultWriteObject();
    s.writeLong(sum());
  }

  private void readObject(ObjectInputStream s) throws IOException, ClassNotFoundException {
    s.defaultReadObject();
    busy = 0;
    cells = null;
    base = s.readLong();
  }
}
