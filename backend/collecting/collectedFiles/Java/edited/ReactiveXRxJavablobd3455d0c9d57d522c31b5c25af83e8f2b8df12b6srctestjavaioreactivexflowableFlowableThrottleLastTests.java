

package io.reactivex.flowable;

import static org.mockito.Mockito.inOrder;

import java.util.concurrent.TimeUnit;

import org.junit.Test;
import org.mockito.InOrder;
import org.reactivestreams.Subscriber;

import io.reactivex.TestHelper;
import io.reactivex.processors.PublishProcessor;
import io.reactivex.schedulers.TestScheduler;

public class FlowableThrottleLastTests {

    @Test
    public void testThrottle() {
        Subscriber<Integer> observer = TestHelper.mockSubscriber();

        TestScheduler s = new TestScheduler();
        PublishProcessor<Integer> o = PublishProcessor.create();
        o.throttleLast(500, TimeUnit.MILLISECONDS, s).subscribe(observer);

                s.advanceTimeTo(0, TimeUnit.MILLISECONDS);
        o.onNext(1);         o.onNext(2);         s.advanceTimeTo(501, TimeUnit.MILLISECONDS);
        o.onNext(3);         s.advanceTimeTo(600, TimeUnit.MILLISECONDS);
        o.onNext(4);         s.advanceTimeTo(700, TimeUnit.MILLISECONDS);
        o.onNext(5);         o.onNext(6);         s.advanceTimeTo(1001, TimeUnit.MILLISECONDS);
        o.onNext(7);         s.advanceTimeTo(1501, TimeUnit.MILLISECONDS);
        o.onComplete();

        InOrder inOrder = inOrder(observer);
        inOrder.verify(observer).onNext(2);
        inOrder.verify(observer).onNext(6);
        inOrder.verify(observer).onNext(7);
        inOrder.verify(observer).onComplete();
        inOrder.verifyNoMoreInteractions();
    }
}
