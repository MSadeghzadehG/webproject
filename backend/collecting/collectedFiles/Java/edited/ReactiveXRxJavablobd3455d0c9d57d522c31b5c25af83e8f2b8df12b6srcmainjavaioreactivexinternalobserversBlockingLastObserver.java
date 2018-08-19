

package io.reactivex.internal.observers;


public final class BlockingLastObserver<T> extends BlockingBaseObserver<T> {

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
