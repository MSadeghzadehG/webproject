

package io.reactivex.subscribers;

import static org.junit.Assert.*;

import java.util.concurrent.atomic.AtomicInteger;

import org.junit.*;
import org.reactivestreams.Subscription;

import io.reactivex.exceptions.*;
import io.reactivex.functions.Consumer;
import io.reactivex.plugins.RxJavaPlugins;

public class SafeSubscriberWithPluginTest {
    private final class SubscriptionCancelThrows implements Subscription {
        @Override
        public void cancel() {
            throw new RuntimeException();
        }

        @Override
        public void request(long n) {

        }
    }

    @Before
    @After
    public void resetBefore() {
        RxJavaPlugins.reset();
    }

    @Test
    @Ignore("Subscribers can't throw")
    public void testOnCompletedThrows() {
        TestSubscriber<Integer> ts = new TestSubscriber<Integer>() {
            @Override
            public void onComplete() {
                throw new TestException();
            }
        };
        SafeSubscriber<Integer> safe = new SafeSubscriber<Integer>(ts);
        try {
            safe.onComplete();
            fail();
        } catch (RuntimeException e) {
                                }
    }

    @Test
    public void testOnCompletedThrows2() {
        TestSubscriber<Integer> ts = new TestSubscriber<Integer>() {
            @Override
            public void onComplete() {
                throw new RuntimeException(new TestException());
            }
        };
        SafeSubscriber<Integer> safe = new SafeSubscriber<Integer>(ts);

        try {
            safe.onComplete();
        } catch (RuntimeException ex) {
                    }

                    }

    @Test(expected = RuntimeException.class)
    @Ignore("Subscribers can't throw")
    public void testPluginException() {
        RxJavaPlugins.setErrorHandler(new Consumer<Throwable>() {
            @Override
            public void accept(Throwable e) {
                throw new RuntimeException();
            }
        });

        TestSubscriber<Integer> ts = new TestSubscriber<Integer>() {
            @Override
            public void onComplete() {
                throw new TestException();
            }
        };
        SafeSubscriber<Integer> safe = new SafeSubscriber<Integer>(ts);

        safe.onComplete();
    }

    @Test(expected = RuntimeException.class)
    @Ignore("Subscribers can't throw")
    public void testPluginExceptionWhileOnErrorUnsubscribeThrows() {
        RxJavaPlugins.setErrorHandler(new Consumer<Throwable>() {
            int calls;
            @Override
            public void accept(Throwable e) {
                if (++calls == 2) {
                    throw new RuntimeException();
                }
            }
        });

        TestSubscriber<Integer> ts = new TestSubscriber<Integer>();
        SafeSubscriber<Integer> safe = new SafeSubscriber<Integer>(ts);
        safe.onSubscribe(new SubscriptionCancelThrows());

        safe.onError(new TestException());
    }

    @Test(expected = RuntimeException.class)
    @Ignore("Subscribers can't throw")
    public void testPluginExceptionWhileOnErrorThrowsNotImplAndUnsubscribeThrows() {
        RxJavaPlugins.setErrorHandler(new Consumer<Throwable>() {
            int calls;
            @Override
            public void accept(Throwable e) {
                if (++calls == 2) {
                    throw new RuntimeException();
                }
            }
        });

        TestSubscriber<Integer> ts = new TestSubscriber<Integer>() {
            @Override
            public void onError(Throwable e) {
                throw new RuntimeException(e);
            }
        };
        SafeSubscriber<Integer> safe = new SafeSubscriber<Integer>(ts);
        safe.onSubscribe(new SubscriptionCancelThrows());

        safe.onError(new TestException());
    }

    @Test(expected = RuntimeException.class)
    @Ignore("Subscribers can't throw")
    public void testPluginExceptionWhileOnErrorThrows() {
        RxJavaPlugins.setErrorHandler(new Consumer<Throwable>() {
            int calls;
            @Override
            public void accept(Throwable e) {
                if (++calls == 2) {
                    throw new RuntimeException();
                }
            }
        });

        TestSubscriber<Integer> ts = new TestSubscriber<Integer>() {
            @Override
            public void onError(Throwable e) {
                throw new RuntimeException(e);
            }
        };
        SafeSubscriber<Integer> safe = new SafeSubscriber<Integer>(ts);

        safe.onError(new TestException());
    }
    @Test(expected = RuntimeException.class)
    @Ignore("Subscribers can't throw")
    public void testPluginExceptionWhileOnErrorThrowsAndUnsubscribeThrows() {
        RxJavaPlugins.setErrorHandler(new Consumer<Throwable>() {
            int calls;
            @Override
            public void accept(Throwable e) {
                if (++calls == 2) {
                    throw new RuntimeException();
                }
            }
        });

        TestSubscriber<Integer> ts = new TestSubscriber<Integer>() {
            @Override
            public void onError(Throwable e) {
                throw new RuntimeException(e);
            }
        };
        SafeSubscriber<Integer> safe = new SafeSubscriber<Integer>(ts);
        safe.onSubscribe(new SubscriptionCancelThrows());

        safe.onError(new TestException());
    }
    @Test(expected = RuntimeException.class)
    @Ignore("Subscribers can't throw")
    public void testPluginExceptionWhenUnsubscribing2() {
        RxJavaPlugins.setErrorHandler(new Consumer<Throwable>() {
            int calls;
            @Override
            public void accept(Throwable e) {
                if (++calls == 3) {
                    throw new RuntimeException();
                }
            }
        });

        TestSubscriber<Integer> ts = new TestSubscriber<Integer>() {
            @Override
            public void onError(Throwable e) {
                throw new RuntimeException(e);
            }
        };
        SafeSubscriber<Integer> safe = new SafeSubscriber<Integer>(ts);
        safe.onSubscribe(new SubscriptionCancelThrows());

        safe.onError(new TestException());
    }

    @Test
    @Ignore("Subscribers can't throw")
    public void testPluginErrorHandlerReceivesExceptionWhenUnsubscribeAfterCompletionThrows() {
        final AtomicInteger calls = new AtomicInteger();
        RxJavaPlugins.setErrorHandler(new Consumer<Throwable>() {
            @Override
            public void accept(Throwable e) {
                calls.incrementAndGet();
            }
        });

        final AtomicInteger errorCount = new AtomicInteger();
        TestSubscriber<Integer> ts = new TestSubscriber<Integer>() {
            @Override
            public void onError(Throwable e) {
                errorCount.incrementAndGet();
            }
        };
        final RuntimeException ex = new RuntimeException();
        SafeSubscriber<Integer> safe = new SafeSubscriber<Integer>(ts);
        safe.onSubscribe(new Subscription() {
            @Override
            public void cancel() {
                throw ex;
            }

            @Override
            public void request(long n) {

            }
        });

    }

    @Test
    @Ignore("Subscribers can't throw")
    public void testPluginErrorHandlerReceivesExceptionFromFailingUnsubscribeAfterCompletionThrows() {
        final AtomicInteger calls = new AtomicInteger();
        RxJavaPlugins.setErrorHandler(new Consumer<Throwable>() {
            @Override
            public void accept(Throwable e) {
                    calls.incrementAndGet();
            }
        });

        final AtomicInteger errorCount = new AtomicInteger();
        TestSubscriber<Integer> ts = new TestSubscriber<Integer>() {

            @Override
            public void onComplete() {
                throw new RuntimeException();
            }

            @Override
            public void onError(Throwable e) {
                errorCount.incrementAndGet();
            }
        };
        SafeSubscriber<Integer> safe = new SafeSubscriber<Integer>(ts);
        safe.onSubscribe(new SubscriptionCancelThrows());

    }
}
