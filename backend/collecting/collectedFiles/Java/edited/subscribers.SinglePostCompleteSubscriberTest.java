

package io.reactivex.internal.subscribers;

import org.junit.Test;

import io.reactivex.TestHelper;
import io.reactivex.internal.subscriptions.BooleanSubscription;
import io.reactivex.subscribers.TestSubscriber;

public class SinglePostCompleteSubscriberTest {

    @Test
    public void requestCompleteRace() {
        for (int i = 0; i < TestHelper.RACE_DEFAULT_LOOPS; i++) {
            final TestSubscriber<Integer> ts = new TestSubscriber<Integer>(0L);

            final SinglePostCompleteSubscriber<Integer, Integer> spc = new SinglePostCompleteSubscriber<Integer, Integer>(ts) {
                private static final long serialVersionUID = -2848918821531562637L;

                @Override
                public void onNext(Integer t) {
                }

                @Override
                public void onError(Throwable t) {
                }

                @Override
                public void onComplete() {
                    complete(1);
                }
            };

            spc.onSubscribe(new BooleanSubscription());

            Runnable r1 = new Runnable() {
                @Override
                public void run() {
                    spc.onComplete();
                }
            };

            Runnable r2 = new Runnable() {
                @Override
                public void run() {
                    ts.request(1);
                }
            };

            TestHelper.race(r1, r2);

            ts.assertResult(1);
        }
    }
}
