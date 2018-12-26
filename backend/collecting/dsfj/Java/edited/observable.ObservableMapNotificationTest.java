

package io.reactivex.internal.operators.observable;

import java.util.concurrent.Callable;

import org.junit.Test;

import io.reactivex.*;
import io.reactivex.disposables.Disposables;
import io.reactivex.exceptions.*;
import io.reactivex.functions.Function;
import io.reactivex.internal.functions.Functions;
import io.reactivex.internal.operators.observable.ObservableMapNotification.MapNotificationObserver;
import io.reactivex.observers.TestObserver;

public class ObservableMapNotificationTest {
    @Test
    public void testJust() {
        TestObserver<Object> to = new TestObserver<Object>();
        Observable.just(1)
        .flatMap(
                new Function<Integer, Observable<Object>>() {
                    @Override
                    public Observable<Object> apply(Integer item) {
                        return Observable.just((Object)(item + 1));
                    }
                },
                new Function<Throwable, Observable<Object>>() {
                    @Override
                    public Observable<Object> apply(Throwable e) {
                        return Observable.error(e);
                    }
                },
                new Callable<Observable<Object>>() {
                    @Override
                    public Observable<Object> call() {
                        return Observable.never();
                    }
                }
        ).subscribe(to);

        to.assertNoErrors();
        to.assertNotComplete();
        to.assertValue(2);
    }

    @Test
    public void dispose() {
        TestHelper.checkDisposed(new Observable<Integer>() {
            @SuppressWarnings({ "rawtypes", "unchecked" })
            @Override
            protected void subscribeActual(Observer<? super Integer> observer) {
                MapNotificationObserver mn = new MapNotificationObserver(
                        observer,
                        Functions.justFunction(Observable.just(1)),
                        Functions.justFunction(Observable.just(2)),
                        Functions.justCallable(Observable.just(3))
                );
                mn.onSubscribe(Disposables.empty());
            }
        });
    }

    @Test
    public void doubleOnSubscribe() {
        TestHelper.checkDoubleOnSubscribeObservable(new Function<Observable<Object>, ObservableSource<Integer>>() {
            @Override
            public ObservableSource<Integer> apply(Observable<Object> o) throws Exception {
                return o.flatMap(
                        Functions.justFunction(Observable.just(1)),
                        Functions.justFunction(Observable.just(2)),
                        Functions.justCallable(Observable.just(3))
                );
            }
        });
    }

    @Test
    public void onErrorCrash() {
        TestObserver<Integer> to = Observable.<Integer>error(new TestException("Outer"))
        .flatMap(Functions.justFunction(Observable.just(1)),
                new Function<Throwable, Observable<Integer>>() {
                    @Override
                    public Observable<Integer> apply(Throwable t) throws Exception {
                        throw new TestException("Inner");
                    }
                },
                Functions.justCallable(Observable.just(3)))
        .test()
        .assertFailure(CompositeException.class);

        TestHelper.assertError(to, 0, TestException.class, "Outer");
        TestHelper.assertError(to, 1, TestException.class, "Inner");
    }
}
