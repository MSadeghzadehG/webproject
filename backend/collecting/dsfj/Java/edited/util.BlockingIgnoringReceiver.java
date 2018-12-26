

package io.reactivex.internal.util;

import java.util.concurrent.CountDownLatch;

import io.reactivex.functions.*;


public final class BlockingIgnoringReceiver
extends CountDownLatch
implements Consumer<Throwable>, Action {
    public Throwable error;

    public BlockingIgnoringReceiver() {
        super(1);
    }

    @Override
    public void accept(Throwable e) {
        error = e;
        countDown();
    }

    @Override
    public void run() {
        countDown();
    }
}
