

package io.reactivex.internal.operators.flowable;

import org.reactivestreams.Subscriber;

import io.reactivex.*;
import io.reactivex.internal.subscribers.SinglePostCompleteSubscriber;
import io.reactivex.plugins.RxJavaPlugins;

public final class FlowableMaterialize<T> extends AbstractFlowableWithUpstream<T, Notification<T>> {

    public FlowableMaterialize(Flowable<T> source) {
        super(source);
    }

    @Override
    protected void subscribeActual(Subscriber<? super Notification<T>> s) {
        source.subscribe(new MaterializeSubscriber<T>(s));
    }

    static final class MaterializeSubscriber<T> extends SinglePostCompleteSubscriber<T, Notification<T>> {

        private static final long serialVersionUID = -3740826063558713822L;

        MaterializeSubscriber(Subscriber<? super Notification<T>> actual) {
            super(actual);
        }

        @Override
        public void onNext(T t) {
            produced++;
            actual.onNext(Notification.createOnNext(t));
        }

        @Override
        public void onError(Throwable t) {
            complete(Notification.<T>createOnError(t));
        }

        @Override
        public void onComplete() {
            complete(Notification.<T>createOnComplete());
        }

        @Override
        protected void onDrop(Notification<T> n) {
            if (n.isOnError()) {
                RxJavaPlugins.onError(n.getError());
            }
        }
    }
}
