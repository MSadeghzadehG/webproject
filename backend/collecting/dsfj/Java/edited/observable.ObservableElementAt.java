

package io.reactivex.internal.operators.observable;

import java.util.NoSuchElementException;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.internal.disposables.DisposableHelper;
import io.reactivex.plugins.RxJavaPlugins;

public final class ObservableElementAt<T> extends AbstractObservableWithUpstream<T, T> {
    final long index;
    final T defaultValue;
    final boolean errorOnFewer;

    public ObservableElementAt(ObservableSource<T> source, long index, T defaultValue, boolean errorOnFewer) {
        super(source);
        this.index = index;
        this.defaultValue = defaultValue;
        this.errorOnFewer = errorOnFewer;
    }
    @Override
    public void subscribeActual(Observer<? super T> t) {
        source.subscribe(new ElementAtObserver<T>(t, index, defaultValue, errorOnFewer));
    }

    static final class ElementAtObserver<T> implements Observer<T>, Disposable {
        final Observer<? super T> actual;
        final long index;
        final T defaultValue;
        final boolean errorOnFewer;

        Disposable s;

        long count;

        boolean done;

        ElementAtObserver(Observer<? super T> actual, long index, T defaultValue, boolean errorOnFewer) {
            this.actual = actual;
            this.index = index;
            this.defaultValue = defaultValue;
            this.errorOnFewer = errorOnFewer;
        }

        @Override
        public void onSubscribe(Disposable s) {
            if (DisposableHelper.validate(this.s, s)) {
                this.s = s;
                actual.onSubscribe(this);
            }
        }


        @Override
        public void dispose() {
            s.dispose();
        }

        @Override
        public boolean isDisposed() {
            return s.isDisposed();
        }


        @Override
        public void onNext(T t) {
            if (done) {
                return;
            }
            long c = count;
            if (c == index) {
                done = true;
                s.dispose();
                actual.onNext(t);
                actual.onComplete();
                return;
            }
            count = c + 1;
        }

        @Override
        public void onError(Throwable t) {
            if (done) {
                RxJavaPlugins.onError(t);
                return;
            }
            done = true;
            actual.onError(t);
        }

        @Override
        public void onComplete() {
            if (!done) {
                done = true;
                T v = defaultValue;
                if (v == null && errorOnFewer) {
                    actual.onError(new NoSuchElementException());
                } else {
                    if (v != null) {
                        actual.onNext(v);
                    }
                    actual.onComplete();
                }
            }
        }
    }
}
