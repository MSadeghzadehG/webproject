

package io.reactivex.internal.operators.completable;

import static org.junit.Assert.*;
import org.junit.Test;

import io.reactivex.*;
import io.reactivex.exceptions.TestException;

public class CompletableLiftTest {

    @Test
    public void callbackThrows() {
        try {
            Completable.complete()
            .lift(new CompletableOperator() {
                @Override
                public CompletableObserver apply(CompletableObserver o) throws Exception {
                    throw new TestException();
                }
            })
            .test();
        } catch (NullPointerException ex) {
            assertTrue(ex.toString(), ex.getCause() instanceof TestException);
        }
    }
}
