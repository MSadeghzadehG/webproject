

package io.reactivex.internal.subscribers;


public interface InnerQueuedSubscriberSupport<T> {

    void innerNext(InnerQueuedSubscriber<T> inner, T value);

    void innerError(InnerQueuedSubscriber<T> inner, Throwable e);

    void innerComplete(InnerQueuedSubscriber<T> inner);

    void drain();
}
