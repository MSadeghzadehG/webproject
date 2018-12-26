

package com.google.common.util.concurrent;

import java.util.concurrent.ExecutionException;
import java.util.concurrent.Future;


public final class Uninterruptibles {

  private Uninterruptibles() {}

  public static <V> V getUninterruptibly(Future<V> future) throws ExecutionException {
    try {
      return future.get();
    } catch (InterruptedException e) {
            throw new IllegalStateException(e);
    }
  }
}
