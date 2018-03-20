

package io.reactivex.internal.operators.completable;

import org.junit.Test;

import io.reactivex.*;
import io.reactivex.functions.Function;
import io.reactivex.schedulers.Schedulers;

public class CompletableObserveOnTest {

    @Test
    public void dispose() {
        TestHelper.checkDisposed(Completable.complete().observeOn(Schedulers.single()));
    }

    @Test
    public void doubleOnSubscribe() {
        TestHelper.checkDoubleOnSubscribeCompletable(new Function<Completable, CompletableSource>() {
            @Override
            public CompletableSource apply(Completable c) throws Exception {
                return c.observeOn(Schedulers.single());
            }
        });
    }
}
