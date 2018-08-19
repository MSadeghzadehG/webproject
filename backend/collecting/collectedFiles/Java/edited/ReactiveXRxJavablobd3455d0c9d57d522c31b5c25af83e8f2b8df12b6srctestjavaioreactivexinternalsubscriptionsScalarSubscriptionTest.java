

package io.reactivex.internal.subscriptions;

import java.util.List;

import org.junit.Test;

import io.reactivex.TestHelper;
import io.reactivex.plugins.RxJavaPlugins;
import io.reactivex.subscribers.TestSubscriber;

public class ScalarSubscriptionTest {

    @Test
    public void badRequest() {
        TestSubscriber<Integer> ts = new TestSubscriber<Integer>(0L);

        ScalarSubscription<Integer> sc = new ScalarSubscription<Integer>(ts, 1);

        List<Throwable> errors = TestHelper.trackPluginErrors();
        try {
            sc.request(-99);

            TestHelper.assertError(errors, 0, IllegalArgumentException.class, "n > 0 required but it was -99");
        } finally {
            RxJavaPlugins.reset();
        }
    }

    @Test
    public void noOffer() {
        TestSubscriber<Integer> ts = new TestSubscriber<Integer>(0L);

        ScalarSubscription<Integer> sc = new ScalarSubscription<Integer>(ts, 1);

        TestHelper.assertNoOffer(sc);
    }
}
