

package org.elasticsearch.threadpool;

import org.elasticsearch.test.ESTestCase;
import org.junit.After;
import org.junit.Before;

import java.util.Optional;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.atomic.AtomicReference;
import java.util.function.Consumer;
import java.util.function.Supplier;

import static org.hamcrest.Matchers.containsString;
import static org.hamcrest.Matchers.hasToString;
import static org.hamcrest.Matchers.instanceOf;

public class EvilThreadPoolTests extends ESTestCase {

    private ThreadPool threadPool;

    @Before
    public void setUpThreadPool() {
        threadPool = new TestThreadPool(EvilThreadPoolTests.class.getName());
    }

    @After
    public void tearDownThreadPool() throws InterruptedException {
        terminate(threadPool);
    }

    public void testExecutionException() throws InterruptedException {
        runExecutionExceptionTest(
                () -> {
                    throw new Error("future error");
                },
                true,
                o -> {
                    assertTrue(o.isPresent());
                    assertThat(o.get(), instanceOf(Error.class));
                    assertThat(o.get(), hasToString(containsString("future error")));
                });
        runExecutionExceptionTest(
                () -> {
                    throw new IllegalStateException("future exception");
                },
                false,
                o -> assertFalse(o.isPresent()));
    }

    private void runExecutionExceptionTest(
            final Runnable runnable,
            final boolean expectThrowable,
            final Consumer<Optional<Throwable>> consumer) throws InterruptedException {
        final AtomicReference<Throwable> throwableReference = new AtomicReference<>();
        final Thread.UncaughtExceptionHandler uncaughtExceptionHandler = Thread.getDefaultUncaughtExceptionHandler();
        final CountDownLatch uncaughtExceptionHandlerLatch = new CountDownLatch(1);

        try {
            Thread.setDefaultUncaughtExceptionHandler((t, e) -> {
                assertTrue(expectThrowable);
                throwableReference.set(e);
                uncaughtExceptionHandlerLatch.countDown();
            });

            final CountDownLatch supplierLatch = new CountDownLatch(1);

            threadPool.generic().submit(() -> {
                try {
                    runnable.run();
                } finally {
                    supplierLatch.countDown();
                }
            });

            supplierLatch.await();

            if (expectThrowable) {
                uncaughtExceptionHandlerLatch.await();
            }
            consumer.accept(Optional.ofNullable(throwableReference.get()));
        } finally {
            Thread.setDefaultUncaughtExceptionHandler(uncaughtExceptionHandler);
        }
    }

}
