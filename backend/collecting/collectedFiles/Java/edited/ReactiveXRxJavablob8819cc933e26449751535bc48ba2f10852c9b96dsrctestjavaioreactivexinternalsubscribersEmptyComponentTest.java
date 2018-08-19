

package io.reactivex.internal.subscribers;

import static org.junit.Assert.assertTrue;

import java.util.List;

import org.junit.Test;

import io.reactivex.TestHelper;
import io.reactivex.disposables.*;
import io.reactivex.exceptions.TestException;
import io.reactivex.internal.subscriptions.BooleanSubscription;
import io.reactivex.internal.util.EmptyComponent;
import io.reactivex.plugins.RxJavaPlugins;

public class EmptyComponentTest {

    @Test
    public void normal() {
        List<Throwable> errors = TestHelper.trackPluginErrors();

        try {
            TestHelper.checkEnum(EmptyComponent.class);

            EmptyComponent c = EmptyComponent.INSTANCE;

            assertTrue(c.isDisposed());

            c.request(10);

            c.request(-10);

            Disposable d = Disposables.empty();

            c.onSubscribe(d);

            assertTrue(d.isDisposed());

            BooleanSubscription s = new BooleanSubscription();

            c.onSubscribe(s);

            assertTrue(s.isCancelled());

            c.onNext(null);

            c.onNext(1);

            c.onComplete();

            c.onError(new TestException());

            c.onSuccess(2);

            c.cancel();

            TestHelper.assertUndeliverable(errors, 0, TestException.class);
        } finally {
            RxJavaPlugins.reset();
        }
    }
}
