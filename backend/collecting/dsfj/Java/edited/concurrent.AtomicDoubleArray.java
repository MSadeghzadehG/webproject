



package com.google.common.util.concurrent;

import static java.lang.Double.doubleToRawLongBits;
import static java.lang.Double.longBitsToDouble;

import com.google.common.annotations.GwtIncompatible;
import com.google.errorprone.annotations.CanIgnoreReturnValue;
import java.util.concurrent.atomic.AtomicLongArray;


@GwtIncompatible
public class AtomicDoubleArray implements java.io.Serializable {
  private static final long serialVersionUID = 0L;

      private transient AtomicLongArray longs;

  
  public AtomicDoubleArray(int length) {
    this.longs = new AtomicLongArray(length);
  }

  
  public AtomicDoubleArray(double[] array) {
    final int len = array.length;
    long[] longArray = new long[len];
    for (int i = 0; i < len; i++) {
      longArray[i] = doubleToRawLongBits(array[i]);
    }
    this.longs = new AtomicLongArray(longArray);
  }

  
  public final int length() {
    return longs.length();
  }

  
  public final double get(int i) {
    return longBitsToDouble(longs.get(i));
  }

  
  public final void set(int i, double newValue) {
    long next = doubleToRawLongBits(newValue);
    longs.set(i, next);
  }

  
  public final void lazySet(int i, double newValue) {
    long next = doubleToRawLongBits(newValue);
    longs.lazySet(i, next);
  }

  
  public final double getAndSet(int i, double newValue) {
    long next = doubleToRawLongBits(newValue);
    return longBitsToDouble(longs.getAndSet(i, next));
  }

  
  public final boolean compareAndSet(int i, double expect, double update) {
    return longs.compareAndSet(i, doubleToRawLongBits(expect), doubleToRawLongBits(update));
  }

  
  public final boolean weakCompareAndSet(int i, double expect, double update) {
    return longs.weakCompareAndSet(i, doubleToRawLongBits(expect), doubleToRawLongBits(update));
  }

  
  @CanIgnoreReturnValue
  public final double getAndAdd(int i, double delta) {
    while (true) {
      long current = longs.get(i);
      double currentVal = longBitsToDouble(current);
      double nextVal = currentVal + delta;
      long next = doubleToRawLongBits(nextVal);
      if (longs.compareAndSet(i, current, next)) {
        return currentVal;
      }
    }
  }

  
  @CanIgnoreReturnValue
  public double addAndGet(int i, double delta) {
    while (true) {
      long current = longs.get(i);
      double currentVal = longBitsToDouble(current);
      double nextVal = currentVal + delta;
      long next = doubleToRawLongBits(nextVal);
      if (longs.compareAndSet(i, current, next)) {
        return nextVal;
      }
    }
  }

  
  public String toString() {
    int iMax = length() - 1;
    if (iMax == -1) {
      return "[]";
    }

        StringBuilder b = new StringBuilder((17 + 2) * (iMax + 1));
    b.append('[');
    for (int i = 0; ; i++) {
      b.append(longBitsToDouble(longs.get(i)));
      if (i == iMax) {
        return b.append(']').toString();
      }
      b.append(',').append(' ');
    }
  }

  
  private void writeObject(java.io.ObjectOutputStream s) throws java.io.IOException {
    s.defaultWriteObject();

        int length = length();
    s.writeInt(length);

        for (int i = 0; i < length; i++) {
      s.writeDouble(get(i));
    }
  }

  
  private void readObject(java.io.ObjectInputStream s)
      throws java.io.IOException, ClassNotFoundException {
    s.defaultReadObject();

        int length = s.readInt();
    this.longs = new AtomicLongArray(length);

        for (int i = 0; i < length; i++) {
      set(i, s.readDouble());
    }
  }
}
