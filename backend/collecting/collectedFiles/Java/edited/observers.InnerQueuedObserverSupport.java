

package io.reactivex.internal.observers;


public interface InnerQueuedObserverSupport<T> {

    void innerNext(InnerQueuedObserver<T> inner, T value);

    void innerError(InnerQueuedObserver<T> inner, Throwable e);

    void innerComplete(InnerQueuedObserver<T> inner);

    void drain();
}
