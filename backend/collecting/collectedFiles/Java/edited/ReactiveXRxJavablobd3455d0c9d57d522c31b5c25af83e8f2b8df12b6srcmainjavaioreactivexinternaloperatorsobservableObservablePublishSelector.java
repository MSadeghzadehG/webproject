

package io.reactivex.internal.operators.observable;

import java.util.concurrent.atomic.AtomicReference;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.exceptions.Exceptions;
import io.reactivex.functions.Function;
import io.reactivex.internal.disposables.*;
import io.reactivex.internal.functions.ObjectHelper;
import io.reactivex.subjects.PublishSubject;


public final class ObservablePublishSelector<T, R> extends AbstractObservableWithUpstream<T, R> {

    final Function<? super Observable<T>, ? extends ObservableSource<R>> selector;

    public ObservablePublishSelector(final ObservableSource<T> source,
                                              final Function<? super Observable<T>, ? extends ObservableSource<R>> selector) {
        super(source);
        this.selector = selector;
    }

    @Override
    protected void subscribeActual(Observer<? super R> observer) {
        PublishSubject<T> subject = PublishSubject.create();

        ObservableSource<? extends R> target;

        try {
            target = ObjectHelper.requireNonNull(selector.apply(subject), "The selector returned a null ObservableSource");
        } catch (Throwable ex) {
            Exceptions.throwIfFatal(ex);
            EmptyDisposable.error(ex, observer);
            return;
        }

        TargetObserver<T, R> o = new TargetObserver<T, R>(observer);

        target.subscribe(o);

        source.subscribe(new SourceObserver<T, R>(subject, o));
    }

    static final class SourceObserver<T, R> implements Observer<T> {

        final PublishSubject<T> subject;

        final AtomicReference<Disposable> target;

        SourceObserver(PublishSubject<T> subject, AtomicReference<Disposable> target) {
            this.subject = subject;
            this.target = target;
        }

        @Override
        public void onSubscribe(Disposable d) {
            DisposableHelper.setOnce(target, d);
        }

        @Override
        public void onNext(T value) {
            subject.onNext(value);
        }

        @Override
        public void onError(Throwable e) {
            subject.onError(e);
        }

        @Override
        public void onComplete() {
            subject.onComplete();
        }
    }

    static final class TargetObserver<T, R>
    extends AtomicReference<Disposable> implements Observer<R>, Disposable {
        private static final long serialVersionUID = 854110278590336484L;

        final Observer<? super R> actual;

        Disposable d;

        TargetObserver(Observer<? super R> actual) {
            this.actual = actual;
        }

        @Override
        public void onSubscribe(Disposable d) {
            if (DisposableHelper.validate(this.d, d)) {
                this.d = d;

                actual.onSubscribe(this);
            }
        }

        @Override
        public void onNext(R value) {
            actual.onNext(value);
        }

        @Override
        public void onError(Throwable e) {
            DisposableHelper.dispose(this);
            actual.onError(e);
        }

        @Override
        public void onComplete() {
            DisposableHelper.dispose(this);
            actual.onComplete();
        }

        @Override
        public void dispose() {
            d.dispose();
            DisposableHelper.dispose(this);
        }

        @Override
        public boolean isDisposed() {
            return d.isDisposed();
        }
    }
}
