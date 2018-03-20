

package io.reactivex.internal.operators.single;

import static org.junit.Assert.*;

import java.util.concurrent.*;

import org.junit.Test;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.exceptions.TestException;
import io.reactivex.functions.*;
import io.reactivex.processors.PublishProcessor;
import io.reactivex.schedulers.Schedulers;

public class SingleUnsubscribeOnTest {

    @Test
    public void normal() throws Exception {
        PublishProcessor<Integer> pp = PublishProcessor.create();

        final String[] name = { null };

        final CountDownLatch cdl = new CountDownLatch(1);

        pp.doOnCancel(new Action() {
            @Override
            public void run() throws Exception {
                name[0] = Thread.currentThread().getName();
                cdl.countDown();
            }
        })
        .single(-99)
        .unsubscribeOn(Schedulers.single())
        .test(true)
        ;

        assertTrue(cdl.await(5, TimeUnit.SECONDS));

        int times = 10;

        while (times-- > 0 && pp.hasSubscribers()) {
            Thread.sleep(100);
        }

        assertFalse(pp.hasSubscribers());

        assertNotEquals(Thread.currentThread().getName(), name[0]);
    }

    @Test
    public void just() {
        Single.just(1)
        .unsubscribeOn(Schedulers.single())
        .test()
        .assertResult(1);
    }

    @Test
    public void error() {
        Single.<Integer>error(new TestException())
        .unsubscribeOn(Schedulers.single())
        .test()
        .assertFailure(TestException.class);
    }

    @Test
    public void dispose() {
        TestHelper.checkDisposed(Single.just(1)
        .unsubscribeOn(Schedulers.single()));
    }

    @Test
    public void doubleOnSubscribe() {
        TestHelper.checkDoubleOnSubscribeSingle(new Function<Single<Object>, SingleSource<Object>>() {
            @Override
            public SingleSource<Object> apply(Single<Object> v) throws Exception {
                return v.unsubscribeOn(Schedulers.single());
            }
        });
    }

    @Test
    public void disposeRace() {
        for (int i = 0; i < TestHelper.RACE_DEFAULT_LOOPS; i++) {
            PublishProcessor<Integer> pp = PublishProcessor.create();

            final Disposable[] ds = { null };
            pp.single(-99).unsubscribeOn(Schedulers.computation())
            .subscribe(new SingleObserver<Integer>() {
                @Override
                public void onSubscribe(Disposable d) {
                    ds[0] = d;
                }

                @Override
                public void onSuccess(Integer value) {

                }

                @Override
                public void onError(Throwable e) {

                }
            });

            Runnable r = new Runnable() {
                @Override
                public void run() {
                    ds[0].dispose();
                }
            };

            TestHelper.race(r, r);
        }
    }
}