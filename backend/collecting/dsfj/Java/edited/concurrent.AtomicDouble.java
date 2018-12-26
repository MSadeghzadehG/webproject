



package com.google.common.util.concurrent;

import static java.lang.Double.doubleToRawLongBits;
import static java.lang.Double.longBitsToDouble;

import com.google.errorprone.annotations.CanIgnoreReturnValue;
import java.util.concurrent.atomic.AtomicLong;


public class AtomicDouble extends Number implements java.io.Serializable {
  private static final long serialVersionUID = 0L;

    private transient AtomicLong value;

  
  public AtomicDouble(double initialValue) {
    value = new AtomicLong(doubleToRawLongBits(initialValue));
  }

  
  public AtomicDouble() {
    this(0.0);
  }

  
  public final double get() {
    return longBitsToDouble(value.get());
  }

  
  public final void set(double newValue) {
    long next = doubleToRawLongBits(newValue);
    value.set(next);
  }

  
  public final void lazySet(double newValue) {
    long next = doubleToRawLongBits(newValue);
    value.lazySet(next);
  }

  
  public final double getAndSet(double newValue) {
    long next = doubleToRawLongBits(newValue);
    return longBitsToDouble(value.getAndSet(next));
  }

  
  public final boolean compareAndSet(double expect, double update) {
    return value.compareAndSet(doubleToRawLongBits(expect), doubleToRawLongBits(update));
  }

  
  public final boolean weakCompareAndSet(double expect, double update) {
    return value.weakCompareAndSet(doubleToRawLongBits(expect), doubleToRawLongBits(update));
  }

  
  @CanIgnoreReturnValue
  public final double getAndAdd(double delta) {
    while (true) {
      long current = value.get();
      double currentVal = longBitsToDouble(current);
      double nextVal = currentVal + delta;
      long next = doubleToRawLongBits(nextVal);
      if (value.compareAndSet(current, next)) {
        return currentVal;
      }
    }
  }

  
  @CanIgnoreReturnValue
  public final double addAndGet(double delta) {
    while (true) {
      long current = value.get();
      double currentVal = longBitsToDouble(current);
      double nextVal = currentVal + delta;
      long next = doubleToRawLongBits(nextVal);
      if (value.compareAndSet(current, next)) {
        return nextVal;
      }
    }
  }

  
  public String toString() {
    return Double.toString(get());
  }

  
  public int intValue() {
    return (int) get();
  }

  
  public long longValue() {
    return (long) get();
  }

  
  public float floatValue() {
    return (float) get();
  }

  
  public double doubleValue() {
    return get();
  }

  
  private void writeObject(java.io.ObjectOutputStream s) throws java.io.IOException {
    s.defaultWriteObject();

    s.writeDouble(get());
  }

  
  private void readObject(java.io.ObjectInputStream s)
      throws java.io.IOException, ClassNotFoundException {
    s.defaultReadObject();
    value = new AtomicLong();
    set(s.readDouble());
  }
}
