
package com.alibaba.dubbo.registry.multicast;

import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.common.utils.NetUtils;
import com.alibaba.dubbo.registry.NotifyListener;

import junit.framework.Assert;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import java.net.MulticastSocket;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.atomic.AtomicReference;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;


public class MulticastRegistryTest {

    String service = "com.alibaba.dubbo.test.injvmServie";
    URL registryUrl = URL.valueOf("multicast:    URL serviceUrl = URL.valueOf("dubbo:            + "?methods=test1,test2");
    URL consumerUrl = URL.valueOf("subscribe:    MulticastRegistry registry = new MulticastRegistry(registryUrl);

    
    @BeforeClass
    public static void setUpBeforeClass() throws Exception {
    }

    
    @Before
    public void setUp() throws Exception {
        registry.register(serviceUrl);
    }

    @Test(expected = IllegalArgumentException.class)
    public void testUrlerror() {
        URL errorUrl = URL.valueOf("multicast:        new MulticastRegistry(errorUrl);
    }

    
    @Test
    public void testRegister() {
        Set<URL> registered = null;
                registered = registry.getRegistered();

        for (int i = 0; i < 2; i++) {
            registry.register(serviceUrl);
            registered = registry.getRegistered();
            assertTrue(registered.contains(serviceUrl));
        }
                registered = registry.getRegistered();
        assertEquals(1, registered.size());
    }

    
    @Test
    public void testSubscribe() {
                final AtomicReference<URL> args = new AtomicReference<URL>();
        registry.subscribe(consumerUrl, new NotifyListener() {

            public void notify(List<URL> urls) {
                                args.set(urls.get(0));
            }
        });
        assertEquals(serviceUrl.toFullString(), args.get().toFullString());
        Map<URL, Set<NotifyListener>> arg = registry.getSubscribed();
        assertEquals(consumerUrl, arg.keySet().iterator().next());

    }

    @Test
    public void testDefaultPort() {
        MulticastRegistry multicastRegistry = new MulticastRegistry(URL.valueOf("multicast:        try {
            MulticastSocket multicastSocket = multicastRegistry.getMutilcastSocket();
            Assert.assertEquals(1234, multicastSocket.getLocalPort());
        } finally {
            multicastRegistry.destroy();
        }
    }

}