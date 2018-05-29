

package io.reactivex.tck;

import java.util.concurrent.TimeUnit;

import org.reactivestreams.Publisher;
import org.testng.annotations.Test;

import io.reactivex.processors.ReplayProcessor;
import io.reactivex.schedulers.Schedulers;

@Test
public class ReplayProcessorTimeBoundAsPublisherTckTest extends BaseTck<Integer> {

    public ReplayProcessorTimeBoundAsPublisherTckTest() {
        super(100);
    }

    @Override
    public Publisher<Integer> createPublisher(final long elements) {
        final ReplayProcessor<Integer> pp = ReplayProcessor.createWithTime(1, TimeUnit.MINUTES, Schedulers.computation());

        Schedulers.io().scheduleDirect(new Runnable() {
            @Override
            public void run() {
                long start = System.currentTimeMillis();
                while (!pp.hasSubscribers()) {
                    try {
                        Thread.sleep(1);
                    } catch (InterruptedException ex) {
                        return;
                    }

                    if (System.currentTimeMillis() - start > 200) {
                        return;
                    }
                }

                for (int i = 0; i < elements; i++) {
                    pp.onNext(i);
                }
                pp.onComplete();
            }
        });
        return pp;
    }
}
