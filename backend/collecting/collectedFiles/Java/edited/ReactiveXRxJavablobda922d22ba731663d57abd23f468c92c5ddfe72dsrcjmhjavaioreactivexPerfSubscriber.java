

package io.reactivex;

import java.util.concurrent.CountDownLatch;

import org.openjdk.jmh.infra.Blackhole;
import org.reactivestreams.Subscription;

public class PerfSubscriber implements FlowableSubscriber<Object> {

    public CountDownLatch latch = new CountDownLatch(1);
    private final Blackhole bh;

    public PerfSubscriber(Blackhole bh) {
        this.bh = bh;
    }

    @Override
    public void onSubscribe(Subscription s) {
        s.request(Long.MAX_VALUE);
    }

    @Override
    public void onComplete() {
        latch.countDown();
    }

    @Override
    public void onError(Throwable e) {
        latch.countDown();
    }

    @Override
    public void onNext(Object t) {
        bh.consume(t);
    }

}
