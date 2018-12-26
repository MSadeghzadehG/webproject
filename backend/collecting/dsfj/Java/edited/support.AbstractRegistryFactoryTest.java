
package com.alibaba.dubbo.registry.support;

import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.common.utils.NetUtils;
import com.alibaba.dubbo.registry.NotifyListener;
import com.alibaba.dubbo.registry.Registry;
import com.alibaba.dubbo.registry.RegistryFactory;

import junit.framework.Assert;
import org.junit.Test;

import java.util.List;


public class AbstractRegistryFactoryTest {

    private RegistryFactory registryFactory = new AbstractRegistryFactory() {

        @Override
        protected Registry createRegistry(final URL url) {
            return new Registry() {

                public URL getUrl() {
                    return url;
                }

                public boolean isAvailable() {
                    return false;
                }

                public void destroy() {
                }

                public void register(URL url) {
                }

                public void unregister(URL url) {
                }

                public void subscribe(URL url, NotifyListener listener) {
                }

                public void unsubscribe(URL url, NotifyListener listener) {
                }

                public List<URL> lookup(URL url) {
                    return null;
                }

            };
        }
    };

    @Test
    public void testRegistryFactoryCache() throws Exception {
        URL url = URL.valueOf("dubbo:        Registry registry1 = registryFactory.getRegistry(url);
        Registry registry2 = registryFactory.getRegistry(url);
        Assert.assertEquals(registry1, registry2);
    }

    @Test
    public void testRegistryFactoryIpCache() throws Exception {
        Registry registry1 = registryFactory.getRegistry(URL.valueOf("dubbo:        Registry registry2 = registryFactory.getRegistry(URL.valueOf("dubbo:        Assert.assertEquals(registry1, registry2);
    }

    @Test
    public void testRegistryFactoryGroupCache() throws Exception {
        Registry registry1 = registryFactory.getRegistry(URL.valueOf("dubbo:        Registry registry2 = registryFactory.getRegistry(URL.valueOf("dubbo:        Assert.assertNotSame(registry1, registry2);
    }

}