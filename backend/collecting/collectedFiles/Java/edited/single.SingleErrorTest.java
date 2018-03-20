

package io.reactivex.internal.operators.single;

import java.util.concurrent.Callable;

import org.junit.Test;

import io.reactivex.Single;
import io.reactivex.exceptions.TestException;

public class SingleErrorTest {

    @Test
    public void errorCallableThrows() {
        Single.error(new Callable<Throwable>() {
            @Override
            public Throwable call() throws Exception {
                throw new TestException();
            }
        })
        .test()
        .assertFailure(TestException.class);
    }
}
