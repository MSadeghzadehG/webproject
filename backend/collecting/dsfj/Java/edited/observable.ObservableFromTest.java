

package io.reactivex.internal.operators.observable;

import static org.junit.Assert.*;

import java.util.concurrent.*;

import org.junit.Test;

import io.reactivex.*;
import io.reactivex.functions.Function;
import io.reactivex.internal.fuseable.*;
import io.reactivex.observers.*;
import io.reactivex.schedulers.Schedulers;

public class ObservableFromTest {

    @Test
    public void fromFutureTimeout() throws Exception {
        Observable.fromFuture(Observable.never()
        .toFuture(), 100, TimeUnit.MILLISECONDS, Schedulers.io())
        .test()
        .awaitDone(5, TimeUnit.SECONDS)
        .assertFailure(TimeoutException.class);
    }

    @Test
    public void fromPublisher() {
        Observable.fromPublisher(Flowable.just(1))
        .test()
        .assertResult(1);
    }

    @Test
    public void just10() {
        Observable.just(1, 2, 3, 4, 5, 6, 7, 8, 9, 10)
        .test()
        .assertResult(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    }

    @Test
    public void fromArrayEmpty() {
        assertSame(Observable.empty(), Observable.fromArray());
    }

    @Test
    public void fromArraySingle() {
        assertTrue(Observable.fromArray(1) instanceof ScalarCallable);
    }

    @Test
    public void fromPublisherDispose() {
        TestHelper.checkDisposed(Flowable.just(1).toObservable());
    }

    @Test
    public void fromPublisherDoubleOnSubscribe() {
        TestHelper.checkDoubleOnSubscribeFlowableToObservable(new Function<Flowable<Object>, ObservableSource<Object>>() {
            @Override
            public ObservableSource<Object> apply(Flowable<Object> f) throws Exception {
                return f.toObservable();
            }
        });
    }

    @Test
    public void fusionRejected() {
        TestObserver<Integer> to = ObserverFusion.newTest(QueueFuseable.ASYNC);

        Observable.fromArray(1, 2, 3)
        .subscribe(to);

        ObserverFusion.assertFusion(to, QueueFuseable.NONE)
        .assertResult(1, 2, 3);
    }
}
