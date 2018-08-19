

package io.reactivex.internal.operators.observable;

import static org.junit.Assert.*;
import org.junit.Test;

import io.reactivex.Observable;
import io.reactivex.internal.functions.Functions;
import io.reactivex.internal.fuseable.HasUpstreamObservableSource;

public class AbstractObservableWithUpstreamTest {

    @SuppressWarnings("unchecked")
    @Test
    public void source() {
        Observable<Integer> o = Observable.just(1);

        assertSame(o, ((HasUpstreamObservableSource<Integer>)o.map(Functions.<Integer>identity())).source());
    }
}
