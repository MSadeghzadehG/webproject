

package io.reactivex.internal.operators.maybe;

import java.util.concurrent.Callable;

import org.junit.Test;

import io.reactivex.Maybe;
import io.reactivex.exceptions.TestException;

public class MaybeErrorTest {

    @Test
    public void errorCallableThrows() {
        Maybe.error(new Callable<Throwable>() {
            @Override
            public Throwable call() throws Exception {
                throw new TestException();
            }
        })
        .test()
        .assertFailure(TestException.class);
    }
}
