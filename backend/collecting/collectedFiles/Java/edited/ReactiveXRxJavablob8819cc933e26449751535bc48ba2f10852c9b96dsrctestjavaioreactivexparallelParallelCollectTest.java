

package io.reactivex.parallel;

import static org.junit.Assert.*;

import java.util.*;
import java.util.concurrent.Callable;

import org.junit.Test;

import io.reactivex.*;
import io.reactivex.exceptions.TestException;
import io.reactivex.functions.*;
import io.reactivex.plugins.RxJavaPlugins;
import io.reactivex.processors.PublishProcessor;
import io.reactivex.subscribers.TestSubscriber;

public class ParallelCollectTest {

    @Test
    public void subscriberCount() {
        ParallelFlowableTest.checkSubscriberCount(Flowable.range(1, 5).parallel()
        .collect(new Callable<List<Integer>>() {
            @Override
            public List<Integer> call() throws Exception {
                return new ArrayList<Integer>();
            }
        }, new BiConsumer<List<Integer>, Integer>() {
            @Override
            public void accept(List<Integer> a, Integer b) throws Exception {
                a.add(b);
            }
        }));
    }

    @SuppressWarnings("unchecked")
    @Test
    public void initialCrash() {
        Flowable.range(1, 5)
        .parallel()
        .collect(new Callable<List<Integer>>() {
            @Override
            public List<Integer> call() throws Exception {
                throw new TestException();
            }
        }, new BiConsumer<List<Integer>, Integer>() {
            @Override
            public void accept(List<Integer> a, Integer b) throws Exception {
                a.add(b);
            }
        })
        .sequential()
        .test()
        .assertFailure(TestException.class);
    }

    @SuppressWarnings("unchecked")
    @Test
    public void reducerCrash() {
        Flowable.range(1, 5)
        .parallel()
        .collect(new Callable<List<Integer>>() {
            @Override
            public List<Integer> call() throws Exception {
                return new ArrayList<Integer>();
            }
        }, new BiConsumer<List<Integer>, Integer>() {
            @Override
            public void accept(List<Integer> a, Integer b) throws Exception {
                if (b == 3) {
                    throw new TestException();
                }
                a.add(b);
            }
        })
        .sequential()
        .test()
        .assertFailure(TestException.class);
    }

    @Test
    public void cancel() {
        PublishProcessor<Integer> pp = PublishProcessor.create();

        TestSubscriber<List<Integer>> ts = pp
        .parallel()
        .collect(new Callable<List<Integer>>() {
            @Override
            public List<Integer> call() throws Exception {
                return new ArrayList<Integer>();
            }
        }, new BiConsumer<List<Integer>, Integer>() {
            @Override
            public void accept(List<Integer> a, Integer b) throws Exception {
                a.add(b);
            }
        })
        .sequential()
        .test();

        assertTrue(pp.hasSubscribers());

        ts.cancel();

        assertFalse(pp.hasSubscribers());
    }

    @SuppressWarnings("unchecked")
    @Test
    public void error() {
        Flowable.<Integer>error(new TestException())
        .parallel()
        .collect(new Callable<List<Integer>>() {
            @Override
            public List<Integer> call() throws Exception {
                return new ArrayList<Integer>();
            }
        }, new BiConsumer<List<Integer>, Integer>() {
            @Override
            public void accept(List<Integer> a, Integer b) throws Exception {
                a.add(b);
            }
        })
        .sequential()
        .test()
        .assertFailure(TestException.class);
    }

    @SuppressWarnings("unchecked")
    @Test
    public void doubleError() {
        List<Throwable> errors = TestHelper.trackPluginErrors();
        try {
            new ParallelInvalid()
            .collect(new Callable<List<Object>>() {
                @Override
                public List<Object> call() throws Exception {
                    return new ArrayList<Object>();
                }
            }, new BiConsumer<List<Object>, Object>() {
                @Override
                public void accept(List<Object> a, Object b) throws Exception {
                    a.add(b);
                }
            })
            .sequential()
            .test()
            .assertFailure(TestException.class);

            assertFalse(errors.isEmpty());
            for (Throwable ex : errors) {
                assertTrue(ex.toString(), ex.getCause() instanceof TestException);
            }
        } finally {
            RxJavaPlugins.reset();
        }
    }
}
