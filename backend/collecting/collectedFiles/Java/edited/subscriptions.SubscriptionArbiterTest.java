

package io.reactivex.internal.subscriptions;

import static org.junit.Assert.*;

import java.util.List;

import org.junit.Test;

import io.reactivex.TestHelper;
import io.reactivex.plugins.RxJavaPlugins;

public class SubscriptionArbiterTest {

    @Test
    public void setSubscriptionMissed() {
        SubscriptionArbiter sa = new SubscriptionArbiter();

        sa.getAndIncrement();

        BooleanSubscription bs1 = new BooleanSubscription();

        sa.setSubscription(bs1);

        BooleanSubscription bs2 = new BooleanSubscription();

        sa.setSubscription(bs2);

        assertTrue(bs1.isCancelled());

        assertFalse(bs2.isCancelled());
    }

    @Test
    public void invalidDeferredRequest() {
        SubscriptionArbiter sa = new SubscriptionArbiter();

        List<Throwable> errors = TestHelper.trackPluginErrors();
        try {
            sa.request(-99);

            TestHelper.assertError(errors, 0, IllegalArgumentException.class, "n > 0 required but it was -99");
        } finally {
            RxJavaPlugins.reset();
        }
    }

    @Test
    public void unbounded() {
        SubscriptionArbiter sa = new SubscriptionArbiter();

        sa.request(Long.MAX_VALUE);

        assertEquals(Long.MAX_VALUE, sa.requested);

        assertTrue(sa.isUnbounded());

        sa.unbounded = false;

        sa.request(Long.MAX_VALUE);

        assertEquals(Long.MAX_VALUE, sa.requested);

        sa.produced(1);

        assertEquals(Long.MAX_VALUE, sa.requested);

        sa.unbounded = false;

        sa.produced(Long.MAX_VALUE);

        assertEquals(Long.MAX_VALUE, sa.requested);
    }

    @Test
    public void cancelled() {
        SubscriptionArbiter sa = new SubscriptionArbiter();
        sa.cancelled = true;

        BooleanSubscription bs1 = new BooleanSubscription();

        sa.missedSubscription.set(bs1);

        sa.getAndIncrement();

        sa.drainLoop();

        assertTrue(bs1.isCancelled());
    }

    @Test
    public void drainUnbounded() {
        SubscriptionArbiter sa = new SubscriptionArbiter();

        sa.getAndIncrement();

        sa.requested = Long.MAX_VALUE;

        sa.drainLoop();
    }

    @Test
    public void drainMissedRequested() {
        SubscriptionArbiter sa = new SubscriptionArbiter();

        sa.getAndIncrement();

        sa.requested = 0;

        sa.missedRequested.set(1);

        sa.drainLoop();

        assertEquals(1, sa.requested);
    }

    @Test
    public void drainMissedRequestedProduced() {
        SubscriptionArbiter sa = new SubscriptionArbiter();

        sa.getAndIncrement();

        sa.requested = 0;

        sa.missedRequested.set(Long.MAX_VALUE);

        sa.missedProduced.set(1);

        sa.drainLoop();

        assertEquals(Long.MAX_VALUE, sa.requested);
    }

    @Test
    public void drainMissedRequestedMoreProduced() {
        List<Throwable> errors = TestHelper.trackPluginErrors();
        try {
            SubscriptionArbiter sa = new SubscriptionArbiter();

            sa.getAndIncrement();

            sa.requested = 0;

            sa.missedRequested.set(1);

            sa.missedProduced.set(2);

            sa.drainLoop();

            assertEquals(0, sa.requested);

            TestHelper.assertError(errors, 0, IllegalStateException.class, "More produced than requested: -1");
        } finally {
            RxJavaPlugins.reset();
        }
    }

    @Test
    public void missedSubscriptionNoPrior() {
        SubscriptionArbiter sa = new SubscriptionArbiter();

        sa.getAndIncrement();

        BooleanSubscription bs1 = new BooleanSubscription();

        sa.missedSubscription.set(bs1);

        sa.drainLoop();

        assertSame(bs1, sa.actual);
    }
}
