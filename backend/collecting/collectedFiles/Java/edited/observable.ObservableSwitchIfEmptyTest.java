

package io.reactivex.internal.operators.observable;

import static org.junit.Assert.*;

import java.util.Arrays;
import java.util.concurrent.atomic.AtomicBoolean;

import org.junit.Test;

import io.reactivex.*;
import io.reactivex.disposables.*;
import io.reactivex.functions.Consumer;
import io.reactivex.observers.DefaultObserver;


public class ObservableSwitchIfEmptyTest {

    @Test
    public void testSwitchWhenNotEmpty() throws Exception {
        final AtomicBoolean subscribed = new AtomicBoolean(false);
        final Observable<Integer> o = Observable.just(4)
                .switchIfEmpty(Observable.just(2)
                .doOnSubscribe(new Consumer<Disposable>() {
                    @Override
                    public void accept(Disposable s) {
                        subscribed.set(true);
                    }
                }));

        assertEquals(4, o.blockingSingle().intValue());
        assertFalse(subscribed.get());
    }

    @Test
    public void testSwitchWhenEmpty() throws Exception {
        final Observable<Integer> o = Observable.<Integer>empty()
                .switchIfEmpty(Observable.fromIterable(Arrays.asList(42)));

        assertEquals(42, o.blockingSingle().intValue());
    }

    @Test
    public void testSwitchTriggerUnsubscribe() throws Exception {

        final Disposable d = Disposables.empty();

        Observable<Long> withProducer = Observable.unsafeCreate(new ObservableSource<Long>() {
            @Override
            public void subscribe(final Observer<? super Long> observer) {
                observer.onSubscribe(d);
                observer.onNext(42L);
            }
        });

        Observable.<Long>empty()
                .switchIfEmpty(withProducer)
                .lift(new ObservableOperator<Long, Long>() {
            @Override
            public Observer<? super Long> apply(final Observer<? super Long> child) {
                return new DefaultObserver<Long>() {
                    @Override
                    public void onComplete() {

                    }

                    @Override
                    public void onError(Throwable e) {

                    }

                    @Override
                    public void onNext(Long aLong) {
                        cancel();
                    }

                };
            }
        }).subscribe();


        assertTrue(d.isDisposed());
            }

    @Test
    public void testSwitchShouldTriggerUnsubscribe() {
        final Disposable d = Disposables.empty();

        Observable.unsafeCreate(new ObservableSource<Long>() {
            @Override
            public void subscribe(final Observer<? super Long> observer) {
                observer.onSubscribe(d);
                observer.onComplete();
            }
        }).switchIfEmpty(Observable.<Long>never()).subscribe();
        assertTrue(d.isDisposed());
    }
}
