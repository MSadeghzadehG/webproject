

package com.google.common.util.concurrent;

import com.google.common.annotations.GwtCompatible;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Future;
import org.checkerframework.checker.nullness.compatqual.NullableDecl;


@GwtCompatible
public interface FutureCallback<V> {
  
  void onSuccess(@NullableDecl V result);

  
  void onFailure(Throwable t);
}
