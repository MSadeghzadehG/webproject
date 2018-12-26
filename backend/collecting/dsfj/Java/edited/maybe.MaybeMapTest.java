

package io.reactivex.internal.operators.maybe;

import org.junit.Test;

import io.reactivex.*;
import io.reactivex.functions.Function;
import io.reactivex.internal.functions.Functions;

public class MaybeMapTest {

    @Test
    public void doubleOnSubscribe() {
        TestHelper.checkDoubleOnSubscribeMaybe(new Function<Maybe<Object>, MaybeSource<Object>>() {
            @Override
            public MaybeSource<Object> apply(Maybe<Object> m) throws Exception {
                return m.map(Functions.identity());
            }
        });
    }
}
