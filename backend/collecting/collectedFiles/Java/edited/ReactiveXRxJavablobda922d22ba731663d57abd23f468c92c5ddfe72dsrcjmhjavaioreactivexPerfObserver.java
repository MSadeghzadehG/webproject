

package io.reactivex;

import java.util.concurrent.CountDownLatch;

import org.openjdk.jmh.infra.Blackhole;

import io.reactivex.disposables.Disposable;

public final class PerfObserver implements Observer<Object> {
    final CountDownLatch cdl;
    final Blackhole bh;
    public PerfObserver(Blackhole bh) {
        this.bh = bh;
        this.cdl = new CountDownLatch(1);
    }
    @Override
    public void onSubscribe(Disposable d) {

    }
    @Override
    public void onNext(Object value) {
        bh.consume(value);
    }
    @Override
    public void onError(Throwable e) {
        e.printStackTrace();
        cdl.countDown();
    }
    @Override
    public void onComplete() {
        cdl.countDown();
    }
}
