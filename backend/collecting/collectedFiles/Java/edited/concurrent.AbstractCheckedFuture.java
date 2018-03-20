

package com.google.common.util.concurrent;

import com.google.common.annotations.Beta;
import com.google.common.annotations.GwtIncompatible;
import com.google.errorprone.annotations.CanIgnoreReturnValue;
import java.util.concurrent.CancellationException;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;


@Beta
@Deprecated
@GwtIncompatible
public abstract class AbstractCheckedFuture<V, X extends Exception>
    extends ForwardingListenableFuture.SimpleForwardingListenableFuture<V>
    implements CheckedFuture<V, X> {
  
  protected AbstractCheckedFuture(ListenableFuture<V> delegate) {
    super(delegate);
  }

  
    protected abstract X mapException(Exception e);

  
  @CanIgnoreReturnValue
  @Override
  public V checkedGet() throws X {
    try {
      return get();
    } catch (InterruptedException e) {
      Thread.currentThread().interrupt();
      throw mapException(e);
    } catch (CancellationException | ExecutionException e) {
      throw mapException(e);
    }
  }

  
  @CanIgnoreReturnValue
  @Override
  public V checkedGet(long timeout, TimeUnit unit) throws TimeoutException, X {
    try {
      return get(timeout, unit);
    } catch (InterruptedException e) {
      Thread.currentThread().interrupt();
      throw mapException(e);
    } catch (CancellationException | ExecutionException e) {
      throw mapException(e);
    }
  }
}
