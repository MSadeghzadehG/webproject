
package io.reactivex.internal.operators.observable;

import java.util.Arrays;
import java.util.List;

import io.reactivex.Observable;
import io.reactivex.Observer;
import io.reactivex.disposables.Disposables;


public final class Burst<T> extends Observable<T> {

    final List<T> items;
    final Throwable error;

    Burst(Throwable error, List<T> items) {
        this.error = error;
        this.items = items;
    }

    @Override
    protected void subscribeActual(final Observer<? super T> observer) {
        observer.onSubscribe(Disposables.empty());
        for (T item: items) {
            observer.onNext(item);
        }
        if (error != null) {
            observer.onError(error);
        } else {
            observer.onComplete();
        }
    }

    @SuppressWarnings("unchecked")
    public static <T> Builder<T> item(T item) {
        return items(item);
    }

    public static <T> Builder<T> items(T... items) {
        return new Builder<T>(Arrays.asList(items));
    }

    public static final class Builder<T> {

        private final List<T> items;
        private Throwable error;

        Builder(List<T> items) {
            this.items = items;
        }

        public Observable<T> error(Throwable e) {
            this.error = e;
            return create();
        }

        public Observable<T> create() {
            return new Burst<T>(error, items);
        }

    }

}
