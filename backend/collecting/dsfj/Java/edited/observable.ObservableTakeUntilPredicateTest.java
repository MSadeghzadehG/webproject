

package io.reactivex.internal.operators.observable;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.*;

import java.util.List;

import org.junit.Test;

import io.reactivex.*;
import io.reactivex.disposables.Disposables;
import io.reactivex.exceptions.TestException;
import io.reactivex.functions.*;
import io.reactivex.internal.functions.Functions;
import io.reactivex.observers.TestObserver;
import io.reactivex.plugins.RxJavaPlugins;
import io.reactivex.subjects.PublishSubject;
;

public class ObservableTakeUntilPredicateTest {
    @Test
    public void takeEmpty() {
        Observer<Object> o = TestHelper.mockObserver();

        Observable.empty().takeUntil(new Predicate<Object>() {
            @Override
            public boolean test(Object v) {
                return true;
            }
        }).subscribe(o);

        verify(o, never()).onNext(any());
        verify(o, never()).onError(any(Throwable.class));
        verify(o).onComplete();
    }
    @Test
    public void takeAll() {
        Observer<Object> o = TestHelper.mockObserver();

        Observable.just(1, 2).takeUntil(new Predicate<Integer>() {
            @Override
            public boolean test(Integer v) {
                return false;
            }
        }).subscribe(o);

        verify(o).onNext(1);
        verify(o).onNext(2);
        verify(o, never()).onError(any(Throwable.class));
        verify(o).onComplete();
    }
    @Test
    public void takeFirst() {
        Observer<Object> o = TestHelper.mockObserver();

        Observable.just(1, 2).takeUntil(new Predicate<Integer>() {
            @Override
            public boolean test(Integer v) {
                return true;
            }
        }).subscribe(o);

        verify(o).onNext(1);
        verify(o, never()).onNext(2);
        verify(o, never()).onError(any(Throwable.class));
        verify(o).onComplete();
    }
    @Test
    public void takeSome() {
        Observer<Object> o = TestHelper.mockObserver();

        Observable.just(1, 2, 3).takeUntil(new Predicate<Integer>() {
            @Override
            public boolean test(Integer t1) {
                return t1 == 2;
            }
        })
        .subscribe(o);

        verify(o).onNext(1);
        verify(o).onNext(2);
        verify(o, never()).onNext(3);
        verify(o, never()).onError(any(Throwable.class));
        verify(o).onComplete();
    }
    @Test
    public void functionThrows() {
        Observer<Object> o = TestHelper.mockObserver();

        Predicate<Integer> predicate = (new Predicate<Integer>() {
            @Override
            public boolean test(Integer t1) {
                    throw new TestException("Forced failure");
            }
        });
        Observable.just(1, 2, 3).takeUntil(predicate).subscribe(o);

        verify(o).onNext(1);
        verify(o, never()).onNext(2);
        verify(o, never()).onNext(3);
        verify(o).onError(any(TestException.class));
        verify(o, never()).onComplete();
    }
    @Test
    public void sourceThrows() {
        Observer<Object> o = TestHelper.mockObserver();

        Observable.just(1)
        .concatWith(Observable.<Integer>error(new TestException()))
        .concatWith(Observable.just(2))
        .takeUntil(new Predicate<Integer>() {
            @Override
            public boolean test(Integer v) {
                return false;
            }
        }).subscribe(o);

        verify(o).onNext(1);
        verify(o, never()).onNext(2);
        verify(o).onError(any(TestException.class));
        verify(o, never()).onComplete();
    }

    @Test
    public void testErrorIncludesLastValueAsCause() {
        TestObserver<String> to = new TestObserver<String>();
        final TestException e = new TestException("Forced failure");
        Predicate<String> predicate = (new Predicate<String>() {
            @Override
            public boolean test(String t) {
                    throw e;
            }
        });
        Observable.just("abc").takeUntil(predicate).subscribe(to);

        to.assertTerminated();
        to.assertNotComplete();
        to.assertError(TestException.class);
            }

    @Test
    public void dispose() {
        TestHelper.checkDisposed(PublishSubject.create().takeUntil(Functions.alwaysFalse()));
    }

    @Test
    public void doubleOnSubscribe() {
        TestHelper.checkDoubleOnSubscribeObservable(new Function<Observable<Object>, ObservableSource<Object>>() {
            @Override
            public ObservableSource<Object> apply(Observable<Object> o) throws Exception {
                return o.takeUntil(Functions.alwaysFalse());
            }
        });
    }

    @Test
    public void badSource() {
        List<Throwable> errors = TestHelper.trackPluginErrors();
        try {
            new Observable<Integer>() {
                @Override
                protected void subscribeActual(Observer<? super Integer> observer) {
                    observer.onSubscribe(Disposables.empty());
                    observer.onComplete();
                    observer.onNext(1);
                    observer.onError(new TestException());
                    observer.onComplete();
                }
            }
            .takeUntil(Functions.alwaysFalse())
            .test()
            .assertResult();

            TestHelper.assertUndeliverable(errors, 0, TestException.class);
        } finally {
            RxJavaPlugins.reset();
        }
    }
}
