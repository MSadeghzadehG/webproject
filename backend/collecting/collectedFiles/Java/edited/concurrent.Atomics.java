

package com.google.common.util.concurrent;

import com.google.common.annotations.GwtIncompatible;
import java.util.concurrent.atomic.AtomicReference;
import java.util.concurrent.atomic.AtomicReferenceArray;
import org.checkerframework.checker.nullness.compatqual.NullableDecl;


@GwtIncompatible
public final class Atomics {
  private Atomics() {}

  
  public static <V> AtomicReference<V> newReference() {
    return new AtomicReference<V>();
  }

  
  public static <V> AtomicReference<V> newReference(@NullableDecl V initialValue) {
    return new AtomicReference<V>(initialValue);
  }

  
  public static <E> AtomicReferenceArray<E> newReferenceArray(int length) {
    return new AtomicReferenceArray<E>(length);
  }

  
  public static <E> AtomicReferenceArray<E> newReferenceArray(E[] array) {
    return new AtomicReferenceArray<E>(array);
  }
}
