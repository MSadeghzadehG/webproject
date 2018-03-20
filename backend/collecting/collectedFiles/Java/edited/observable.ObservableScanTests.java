

package io.reactivex.observable;

import java.util.HashMap;

import org.junit.Test;

import io.reactivex.functions.*;
import io.reactivex.observable.ObservableEventStream.Event;

public class ObservableScanTests {

    @Test
    public void testUnsubscribeScan() throws Exception {

        ObservableEventStream.getEventStream("HTTP-ClusterB", 20)
        .scan(new HashMap<String, String>(), new BiFunction<HashMap<String, String>, Event, HashMap<String, String>>() {
            @Override
            public HashMap<String, String> apply(HashMap<String, String> accum, Event perInstanceEvent) {
                accum.put("instance", perInstanceEvent.instanceId);
                return accum;
            }
        })
        .take(10)
        .blockingForEach(new Consumer<HashMap<String, String>>() {
            @Override
            public void accept(HashMap<String, String> pv) {
                System.out.println(pv);
            }
        });

        Thread.sleep(200);     }
}
