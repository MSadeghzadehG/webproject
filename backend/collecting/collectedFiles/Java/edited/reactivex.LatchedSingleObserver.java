

package io.reactivex;

import java.util.concurrent.CountDownLatch;

import org.openjdk.jmh.infra.Blackhole;

import io.reactivex.disposables.Disposable;

public final class LatchedSingleObserver<T> implements SingleObserver<T> {
    final CountDownLatch cdl;
    final Blackhole bh;
    public LatchedSingleObserver(Blackhole bh) {
        this.bh = bh;
        this.cdl = new CountDownLatch(1);
    }
    @Override
    public void onSubscribe(Disposable d) {

    }
    @Override
    public void onSuccess(T value) {
        bh.consume(value);
        cdl.countDown();
    }
    @Override
    public void onError(Throwable e) {
        e.printStackTrace();
        cdl.countDown();
    }
}
