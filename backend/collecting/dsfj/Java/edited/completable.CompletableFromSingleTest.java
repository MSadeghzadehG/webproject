

package io.reactivex.internal.operators.completable;

import io.reactivex.Completable;
import io.reactivex.Single;
import org.junit.Test;

public class CompletableFromSingleTest {
    @Test(expected = NullPointerException.class)
    public void fromSingleNull() {
        Completable.fromSingle(null);
    }

    @Test
    public void fromSingle() {
        Completable.fromSingle(Single.just(1))
            .test()
            .assertResult();
    }

    @Test
    public void fromSingleError() {
        Completable.fromSingle(Single.error(new UnsupportedOperationException()))
            .test()
            .assertFailure(UnsupportedOperationException.class);
    }
}
