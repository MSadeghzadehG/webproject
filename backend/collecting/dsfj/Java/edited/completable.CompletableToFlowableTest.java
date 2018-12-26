

package io.reactivex.internal.operators.completable;

import org.junit.Test;
import org.reactivestreams.Publisher;

import io.reactivex.*;
import io.reactivex.functions.Function;

public class CompletableToFlowableTest {

    @Test
    public void doubleOnSubscribe() {
        TestHelper.checkDoubleOnSubscribeCompletableToFlowable(new Function<Completable, Publisher<?>>() {
            @Override
            public Publisher<?> apply(Completable c) throws Exception {
                return c.toFlowable();
            }
        });
    }
}
