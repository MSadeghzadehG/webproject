

package io.reactivex;

import java.util.concurrent.CountDownLatch;

import org.openjdk.jmh.infra.Blackhole;
import org.reactivestreams.Subscription;


public class PerfBoundedSubscriber extends CountDownLatch implements FlowableSubscriber<Object> {

    final Blackhole bh;

    final long request;

    public PerfBoundedSubscriber(Blackhole bh, long request) {
        super(1);
        this.bh = bh;
        this.request = request;
    }

    @Override
    public void onSubscribe(Subscription s) {
        s.request(request);
    }

    @Override
    public void onComplete() {
        countDown();
    }

    @Override
    public void onError(Throwable e) {
        countDown();
    }

    @Override
    public void onNext(Object t) {
        bh.consume(t);
    }

}
