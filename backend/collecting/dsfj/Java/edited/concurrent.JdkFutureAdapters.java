

package com.google.common.util.concurrent;

import static com.google.common.base.Preconditions.checkNotNull;
import static com.google.common.util.concurrent.Uninterruptibles.getUninterruptibly;

import com.google.common.annotations.Beta;
import com.google.common.annotations.GwtIncompatible;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.atomic.AtomicBoolean;


@Beta
@GwtIncompatible
public final class JdkFutureAdapters {
  
  public static <V> ListenableFuture<V> listenInPoolThread(Future<V> future) {
    if (future instanceof ListenableFuture) {
      return (ListenableFuture<V>) future;
    }
    return new ListenableFutureAdapter<V>(future);
  }

  
  public static <V> ListenableFuture<V> listenInPoolThread(Future<V> future, Executor executor) {
    checkNotNull(executor);
    if (future instanceof ListenableFuture) {
      return (ListenableFuture<V>) future;
    }
    return new ListenableFutureAdapter<V>(future, executor);
  }

  
  private static class ListenableFutureAdapter<V> extends ForwardingFuture<V>
      implements ListenableFuture<V> {

    private static final ThreadFactory threadFactory =
        new ThreadFactoryBuilder()
            .setDaemon(true)
            .setNameFormat("ListenableFutureAdapter-thread-%d")
            .build();
    private static final Executor defaultAdapterExecutor =
        Executors.newCachedThreadPool(threadFactory);

    private final Executor adapterExecutor;

        private final ExecutionList executionList = new ExecutionList();

            private final AtomicBoolean hasListeners = new AtomicBoolean(false);

        private final Future<V> delegate;

    ListenableFutureAdapter(Future<V> delegate) {
      this(delegate, defaultAdapterExecutor);
    }

    ListenableFutureAdapter(Future<V> delegate, Executor adapterExecutor) {
      this.delegate = checkNotNull(delegate);
      this.adapterExecutor = checkNotNull(adapterExecutor);
    }

    @Override
    protected Future<V> delegate() {
      return delegate;
    }

    @Override
    public void addListener(Runnable listener, Executor exec) {
      executionList.add(listener, exec);

                  if (hasListeners.compareAndSet(false, true)) {
        if (delegate.isDone()) {
                              executionList.execute();
          return;
        }

                adapterExecutor.execute(
            new Runnable() {
              @Override
              public void run() {
                try {
                  
                  getUninterruptibly(delegate);
                } catch (Throwable e) {
                                                    }
                executionList.execute();
              }
            });
      }
    }
  }

  private JdkFutureAdapters() {}
}
