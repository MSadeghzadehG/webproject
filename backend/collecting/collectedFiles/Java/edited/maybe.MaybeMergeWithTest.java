

package io.reactivex.internal.operators.maybe;

import org.junit.Test;

import io.reactivex.Maybe;

public class MaybeMergeWithTest {

    @Test
    public void normal() {
        Maybe.just(1).mergeWith(Maybe.just(2))
        .test()
        .assertResult(1, 2);
    }
}
