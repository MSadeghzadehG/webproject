

package io.reactivex.internal.subscribers;


public final class BlockingLastSubscriber<T> extends BlockingBaseSubscriber<T> {

    @Override
    public void onNext(T t) {
        value = t;
    }

    @Override
    public void onError(Throwable t) {
        value = null;
        error = t;
        countDown();
    }
}
