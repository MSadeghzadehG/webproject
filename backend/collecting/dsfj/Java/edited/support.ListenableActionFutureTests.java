
package org.elasticsearch.action.support;

import org.elasticsearch.action.ActionListener;
import org.elasticsearch.common.util.concurrent.AbstractRunnable;
import org.elasticsearch.test.ESTestCase;
import org.elasticsearch.threadpool.TestThreadPool;
import org.elasticsearch.threadpool.ThreadPool;
import org.elasticsearch.transport.Transports;

import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicReference;

public class ListenableActionFutureTests extends ESTestCase {

    public void testListenerIsCallableFromNetworkThreads() throws Throwable {
        ThreadPool threadPool = new TestThreadPool("testListenerIsCallableFromNetworkThreads");
        try {
            final PlainListenableActionFuture<Object> future;
            if (randomBoolean()) {
                future = PlainListenableActionFuture.newDispatchingListenableFuture(threadPool);
            } else {
                future = PlainListenableActionFuture.newListenableFuture();
            }
            final CountDownLatch listenerCalled = new CountDownLatch(1);
            final AtomicReference<Throwable> error = new AtomicReference<>();
            final Object response = new Object();
            future.addListener(new ActionListener<Object>() {
                @Override
                public void onResponse(Object o) {
                    listenerCalled.countDown();
                }

                @Override
                public void onFailure(Exception e) {
                    error.set(e);
                    listenerCalled.countDown();
                }
            });
            Thread networkThread = new Thread(new AbstractRunnable() {
                @Override
                public void onFailure(Exception e) {
                    error.set(e);
                    listenerCalled.countDown();
                }

                @Override
                protected void doRun() throws Exception {
                    future.onResponse(response);
                }
            }, Transports.TEST_MOCK_TRANSPORT_THREAD_PREFIX + "_testListenerIsCallableFromNetworkThread");
            networkThread.start();
            networkThread.join();
            listenerCalled.await();
            if (error.get() != null) {
                throw error.get();
            }
        } finally {
            ThreadPool.terminate(threadPool, 10, TimeUnit.SECONDS);
        }
    }


}
