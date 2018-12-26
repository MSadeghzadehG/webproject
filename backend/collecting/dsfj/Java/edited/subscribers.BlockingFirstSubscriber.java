

package io.reactivex.internal.subscribers;

import io.reactivex.plugins.RxJavaPlugins;


public final class BlockingFirstSubscriber<T> extends BlockingBaseSubscriber<T> {

    @Override
    public void onNext(T t) {
        if (value == null) {
            value = t;
            s.cancel();
            countDown();
        }
    }

    @Override
    public void onError(Throwable t) {
        if (value == null) {
            error = t;
        } else {
            RxJavaPlugins.onError(t);
        }
        countDown();
    }
}
