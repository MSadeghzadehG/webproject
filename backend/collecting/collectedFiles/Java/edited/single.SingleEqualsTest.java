

package io.reactivex.internal.operators.single;

import java.util.List;

import org.junit.Test;

import io.reactivex.*;
import io.reactivex.exceptions.TestException;
import io.reactivex.plugins.RxJavaPlugins;

public class SingleEqualsTest {

    @Test
    public void bothError() {
        List<Throwable> errors = TestHelper.trackPluginErrors();
        try {
            Single.equals(Single.error(new TestException("One")), Single.error(new TestException("Two")))
            .test()
            .assertFailureAndMessage(TestException.class, "One");

            TestHelper.assertUndeliverable(errors, 0, TestException.class, "Two");
        } finally {
            RxJavaPlugins.reset();
        }
    }
}
