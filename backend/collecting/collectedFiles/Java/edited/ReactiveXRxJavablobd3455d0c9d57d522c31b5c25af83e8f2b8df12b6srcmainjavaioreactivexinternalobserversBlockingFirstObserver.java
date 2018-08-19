

package io.reactivex.internal.observers;


public final class BlockingFirstObserver<T> extends BlockingBaseObserver<T> {

    @Override
    public void onNext(T t) {
        if (value == null) {
            value = t;
            d.dispose();
            countDown();
        }
    }

    @Override
    public void onError(Throwable t) {
        if (value == null) {
            error = t;
        }
        countDown();
    }
}
