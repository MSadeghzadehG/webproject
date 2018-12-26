

package io.reactivex.internal.operators.maybe;

import org.junit.Test;

import io.reactivex.*;
import io.reactivex.schedulers.Schedulers;

public class MaybeSubscribeOnTest {

    @Test
    public void dispose() {
        TestHelper.checkDisposed(Maybe.just(1).subscribeOn(Schedulers.single()));
    }
}
