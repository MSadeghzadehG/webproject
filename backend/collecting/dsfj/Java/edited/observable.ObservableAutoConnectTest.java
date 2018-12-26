

package io.reactivex.internal.operators.observable;

import static org.junit.Assert.*;
import org.junit.Test;

import io.reactivex.subjects.PublishSubject;

public class ObservableAutoConnectTest {

    @Test
    public void autoConnectImmediately() {
        PublishSubject<Integer> ps = PublishSubject.create();

        ps.publish().autoConnect(0);

        assertTrue(ps.hasObservers());
    }
}
