
package com.alibaba.dubbo.registry.support;

import com.alibaba.dubbo.common.Constants;
import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.common.utils.CollectionUtils;
import com.alibaba.dubbo.registry.NotifyListener;

import org.junit.Before;
import org.junit.Test;

import java.util.Arrays;
import java.util.List;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicReference;

import static org.junit.Assert.assertEquals;

public class FailbackRegistryTest {
    static String service;
    static URL serviceUrl;
    static URL registryUrl;
    MockRegistry registry;
    private int FAILED_PERIOD = 200;
    private int sleeptime = 100;
    private int trytimes = 5;

    
    @Before
    public void setUp() throws Exception {
        service = "com.alibaba.dubbo.test.DemoService";
        serviceUrl = URL.valueOf("remote:        registryUrl = URL.valueOf("http:    }

    
    @Test
    public void testDoRetry() throws Exception {

        final AtomicReference<Boolean> notified = new AtomicReference<Boolean>(false);
        final CountDownLatch latch = new CountDownLatch(3);
        NotifyListener listner = new NotifyListener() {
            public void notify(List<URL> urls) {
                notified.set(Boolean.TRUE);
            }
        };
        registry = new MockRegistry(registryUrl, latch);
        registry.setBad(true);
        registry.register(serviceUrl);
        registry.unregister(serviceUrl);
        registry.subscribe(serviceUrl.setProtocol(Constants.CONSUMER_PROTOCOL).addParameters(CollectionUtils.toStringMap("check", "false")), listner);
        registry.unsubscribe(serviceUrl.setProtocol(Constants.CONSUMER_PROTOCOL).addParameters(CollectionUtils.toStringMap("check", "false")), listner);

                assertEquals(false, notified.get());
        assertEquals(3, latch.getCount());

        registry.setBad(false);

        for (int i = 0; i < trytimes; i++) {
            System.out.println("failback registry retry ,times:" + i);
                        if (latch.getCount() == 0)
                break;
            Thread.sleep(sleeptime);
        }
        assertEquals(0, latch.getCount());
                assertEquals(false, notified.get());
    }

    @Test
    public void testDoRetry_subscribe() throws Exception {

        final CountDownLatch latch = new CountDownLatch(1);
        registry = new MockRegistry(registryUrl, latch);
        registry.setBad(true);
        registry.register(serviceUrl);

        registry.setBad(false);

        for (int i = 0; i < trytimes; i++) {
            System.out.println("failback registry retry ,times:" + i);
            if (latch.getCount() == 0)
                break;
            Thread.sleep(sleeptime);
        }
        assertEquals(0, latch.getCount());
    }

    @Test
    public void testDoRetry_register() throws Exception {

        final AtomicReference<Boolean> notified = new AtomicReference<Boolean>(false);
        final CountDownLatch latch = new CountDownLatch(1);
        NotifyListener listner = new NotifyListener() {
            public void notify(List<URL> urls) {
                notified.set(Boolean.TRUE);
            }
        };
        registry = new MockRegistry(registryUrl, latch);
        registry.setBad(true);
        registry.subscribe(serviceUrl.setProtocol(Constants.CONSUMER_PROTOCOL).addParameters(CollectionUtils.toStringMap("check", "false")), listner);

                assertEquals(false, notified.get());
        assertEquals(1, latch.getCount());

        registry.setBad(false);

        for (int i = 0; i < trytimes; i++) {
            System.out.println("failback registry retry ,times:" + i);
                        if (latch.getCount() == 0)
                break;
            Thread.sleep(sleeptime);
        }
        assertEquals(0, latch.getCount());
                assertEquals(true, notified.get());
    }

    @Test
    public void testDoRetry_nofify() throws Exception {

                final AtomicInteger count = new AtomicInteger(0);

        NotifyListener listner = new NotifyListener() {
            public void notify(List<URL> urls) {
                count.incrementAndGet();
                                if (count.get() == 1l) {
                    throw new RuntimeException("test exception please ignore");
                }
            }
        };
        registry = new MockRegistry(registryUrl, new CountDownLatch(0));
        registry.subscribe(serviceUrl.setProtocol(Constants.CONSUMER_PROTOCOL).addParameters(CollectionUtils.toStringMap("check", "false")), listner);

        assertEquals(1, count.get());                 for (int i = 0; i < trytimes; i++) {
            System.out.println("failback notify retry ,times:" + i);
            if (count.get() == 2)
                break;
            Thread.sleep(sleeptime);
        }
        assertEquals(2, count.get());
    }


    private static class MockRegistry extends FailbackRegistry {
        CountDownLatch latch;
        private boolean bad = false;

        
        public MockRegistry(URL url, CountDownLatch latch) {
            super(url);
            this.latch = latch;
        }

        
        public void setBad(boolean bad) {
            this.bad = bad;
        }

        @Override
        protected void doRegister(URL url) {
            if (bad) {
                throw new RuntimeException("can not invoke!");
            }
                        latch.countDown();

        }

        @Override
        protected void doUnregister(URL url) {
            if (bad) {
                throw new RuntimeException("can not invoke!");
            }
                        latch.countDown();

        }

        @Override
        protected void doSubscribe(URL url, NotifyListener listener) {
            if (bad) {
                throw new RuntimeException("can not invoke!");
            }
                        super.notify(url, listener, Arrays.asList(new URL[]{serviceUrl}));
            latch.countDown();
        }

        @Override
        protected void doUnsubscribe(URL url, NotifyListener listener) {
            if (bad) {
                throw new RuntimeException("can not invoke!");
            }
                        latch.countDown();
        }

        @Override
        protected void retry() {
            super.retry();
            if (bad) {
                throw new RuntimeException("can not invoke!");
            }
                        latch.countDown();
        }

        public boolean isAvailable() {
            return true;
        }

    }
}
