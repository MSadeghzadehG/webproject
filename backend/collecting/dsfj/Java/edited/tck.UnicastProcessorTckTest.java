

package io.reactivex.tck;

import java.util.concurrent.*;

import org.reactivestreams.*;
import org.reactivestreams.tck.*;
import org.testng.annotations.Test;

import io.reactivex.exceptions.TestException;
import io.reactivex.processors.UnicastProcessor;

@Test
public class UnicastProcessorTckTest extends IdentityProcessorVerification<Integer> {

    public UnicastProcessorTckTest() {
        super(new TestEnvironment(50));
    }

    @Override
    public Processor<Integer, Integer> createIdentityProcessor(int bufferSize) {
        UnicastProcessor<Integer> up = UnicastProcessor.create();
        return new RefCountProcessor<Integer>(up);
    }

    @Override
    public Publisher<Integer> createFailedPublisher() {
        UnicastProcessor<Integer> up = UnicastProcessor.create();
        up.onError(new TestException());
        return up;
    }

    @Override
    public ExecutorService publisherExecutorService() {
        return Executors.newCachedThreadPool();
    }

    @Override
    public Integer createElement(int element) {
        return element;
    }

    @Override
    public long maxSupportedSubscribers() {
        return 1;
    }

    @Override
    public long maxElementsFromPublisher() {
        return 1024;
    }
}
