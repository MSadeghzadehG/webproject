

package io.reactivex.internal.operators.observable;

import java.util.concurrent.atomic.AtomicInteger;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.internal.disposables.SequentialDisposable;

public final class ObservableRepeat<T> extends AbstractObservableWithUpstream<T, T> {
    final long count;
    public ObservableRepeat(Observable<T> source, long count) {
        super(source);
        this.count = count;
    }

    @Override
    public void subscribeActual(Observer<? super T> s) {
        SequentialDisposable sd = new SequentialDisposable();
        s.onSubscribe(sd);

        RepeatObserver<T> rs = new RepeatObserver<T>(s, count != Long.MAX_VALUE ? count - 1 : Long.MAX_VALUE, sd, source);
        rs.subscribeNext();
    }

    static final class RepeatObserver<T> extends AtomicInteger implements Observer<T> {

        private static final long serialVersionUID = -7098360935104053232L;

        final Observer<? super T> actual;
        final SequentialDisposable sd;
        final ObservableSource<? extends T> source;
        long remaining;
        RepeatObserver(Observer<? super T> actual, long count, SequentialDisposable sd, ObservableSource<? extends T> source) {
            this.actual = actual;
            this.sd = sd;
            this.source = source;
            this.remaining = count;
        }

        @Override
        public void onSubscribe(Disposable s) {
            sd.replace(s);
        }

        @Override
        public void onNext(T t) {
            actual.onNext(t);
        }
        @Override
        public void onError(Throwable t) {
            actual.onError(t);
        }

        @Override
        public void onComplete() {
            long r = remaining;
            if (r != Long.MAX_VALUE) {
                remaining = r - 1;
            }
            if (r != 0L) {
                subscribeNext();
            } else {
                actual.onComplete();
            }
        }

        
        void subscribeNext() {
            if (getAndIncrement() == 0) {
                int missed = 1;
                for (;;) {
                    if (sd.isDisposed()) {
                        return;
                    }
                    source.subscribe(this);

                    missed = addAndGet(-missed);
                    if (missed == 0) {
                        break;
                    }
                }
            }
        }
    }
}
