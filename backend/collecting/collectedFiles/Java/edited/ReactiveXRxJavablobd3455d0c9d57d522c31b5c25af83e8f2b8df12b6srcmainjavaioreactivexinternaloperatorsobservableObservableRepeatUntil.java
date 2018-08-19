

package io.reactivex.internal.operators.observable;

import java.util.concurrent.atomic.AtomicInteger;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.exceptions.Exceptions;
import io.reactivex.functions.BooleanSupplier;
import io.reactivex.internal.disposables.SequentialDisposable;

public final class ObservableRepeatUntil<T> extends AbstractObservableWithUpstream<T, T> {
    final BooleanSupplier until;
    public ObservableRepeatUntil(Observable<T> source, BooleanSupplier until) {
        super(source);
        this.until = until;
    }

    @Override
    public void subscribeActual(Observer<? super T> s) {
        SequentialDisposable sd = new SequentialDisposable();
        s.onSubscribe(sd);

        RepeatUntilObserver<T> rs = new RepeatUntilObserver<T>(s, until, sd, source);
        rs.subscribeNext();
    }

    static final class RepeatUntilObserver<T> extends AtomicInteger implements Observer<T> {

        private static final long serialVersionUID = -7098360935104053232L;

        final Observer<? super T> actual;
        final SequentialDisposable sd;
        final ObservableSource<? extends T> source;
        final BooleanSupplier stop;
        RepeatUntilObserver(Observer<? super T> actual, BooleanSupplier until, SequentialDisposable sd, ObservableSource<? extends T> source) {
            this.actual = actual;
            this.sd = sd;
            this.source = source;
            this.stop = until;
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
            boolean b;
            try {
                b = stop.getAsBoolean();
            } catch (Throwable e) {
                Exceptions.throwIfFatal(e);
                actual.onError(e);
                return;
            }
            if (b) {
                actual.onComplete();
            } else {
                subscribeNext();
            }
        }

        
        void subscribeNext() {
            if (getAndIncrement() == 0) {
                int missed = 1;
                for (;;) {
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
