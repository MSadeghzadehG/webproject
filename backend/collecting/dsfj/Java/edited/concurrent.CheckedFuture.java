

package com.google.common.util.concurrent;

import com.google.common.annotations.Beta;
import com.google.common.annotations.GwtCompatible;
import com.google.errorprone.annotations.CanIgnoreReturnValue;
import java.util.concurrent.CancellationException;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;


@Beta
@CanIgnoreReturnValue
@Deprecated
@GwtCompatible
public interface CheckedFuture<V, X extends Exception> extends ListenableFuture<V> {

  
  V checkedGet() throws X;

  
  V checkedGet(long timeout, TimeUnit unit) throws TimeoutException, X;
}
