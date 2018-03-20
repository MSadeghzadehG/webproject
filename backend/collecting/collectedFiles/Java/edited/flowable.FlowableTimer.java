

package io.reactivex.internal.operators.flowable;

import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicReference;

import org.reactivestreams.*;

import io.reactivex.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.exceptions.MissingBackpressureException;
import io.reactivex.internal.disposables.*;
import io.reactivex.internal.subscriptions.SubscriptionHelper;

public final class FlowableTimer extends Flowable<Long> {
    final Scheduler scheduler;
    final long delay;
    final TimeUnit unit;
    public FlowableTimer(long delay, TimeUnit unit, Scheduler scheduler) {
        this.delay = delay;
        this.unit = unit;
        this.scheduler = scheduler;
    }

    @Override
    public void subscribeActual(Subscriber<? super Long> s) {
        TimerSubscriber ios = new TimerSubscriber(s);
        s.onSubscribe(ios);

        Disposable d = scheduler.scheduleDirect(ios, delay, unit);

        ios.setResource(d);
    }

    static final class TimerSubscriber extends AtomicReference<Disposable>
    implements Subscription, Runnable {

        private static final long serialVersionUID = -2809475196591179431L;

        final Subscriber<? super Long> actual;

        volatile boolean requested;

        TimerSubscriber(Subscriber<? super Long> actual) {
            this.actual = actual;
        }

        @Override
        public void request(long n) {
            if (SubscriptionHelper.validate(n)) {
                requested = true;
            }
        }

        @Override
        public void cancel() {
            DisposableHelper.dispose(this);
        }

        @Override
        public void run() {
            if (get() != DisposableHelper.DISPOSED) {
                if (requested) {
                    actual.onNext(0L);
                    lazySet(EmptyDisposable.INSTANCE);
                    actual.onComplete();
                } else {
                    lazySet(EmptyDisposable.INSTANCE);
                    actual.onError(new MissingBackpressureException("Can't deliver value due to lack of requests"));
                }
            }
        }

        public void setResource(Disposable d) {
            DisposableHelper.trySet(this, d);
        }
    }
}
