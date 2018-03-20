
package com.alibaba.dubbo.monitor.support;

import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.common.utils.NetUtils;
import com.alibaba.dubbo.monitor.Monitor;
import com.alibaba.dubbo.monitor.MonitorFactory;

import junit.framework.Assert;
import org.junit.Test;

import java.util.List;


public class AbstractMonitorFactoryTest {

    private MonitorFactory monitorFactory = new AbstractMonitorFactory() {

        protected Monitor createMonitor(final URL url) {
            return new Monitor() {

                public URL getUrl() {
                    return url;
                }

                public boolean isAvailable() {
                    return true;
                }

                public void destroy() {
                }

                public void collect(URL statistics) {
                }

                public List<URL> lookup(URL query) {
                    return null;
                }

            };
        }
    };

    @Test
    public void testMonitorFactoryCache() throws Exception {
        URL url = URL.valueOf("dubbo:        Monitor monitor1 = monitorFactory.getMonitor(url);
        Monitor monitor2 = monitorFactory.getMonitor(url);
        if (monitor1 == null || monitor2 == null) {
            Thread.sleep(2000);
            monitor1 = monitorFactory.getMonitor(url);
            monitor2 = monitorFactory.getMonitor(url);
        }
        Assert.assertEquals(monitor1, monitor2);
    }

    @Test
    public void testMonitorFactoryIpCache() throws Exception {
        URL url = URL.valueOf("dubbo:        Monitor monitor1 = monitorFactory.getMonitor(url);
        Monitor monitor2 = monitorFactory.getMonitor(url);
        if (monitor1 == null || monitor2 == null) {
            Thread.sleep(2000);
            monitor1 = monitorFactory.getMonitor(url);
            monitor2 = monitorFactory.getMonitor(url);
        }
        Assert.assertEquals(monitor1, monitor2);
    }

    @Test
    public void testMonitorFactoryGroupCache() throws Exception {
        URL url1 = URL.valueOf("dubbo:        URL url2 = URL.valueOf("dubbo:        Monitor monitor1 = monitorFactory.getMonitor(url1);
        Monitor monitor2 = monitorFactory.getMonitor(url2);
        if (monitor1 == null || monitor2 == null) {
            Thread.sleep(2000);
            monitor1 = monitorFactory.getMonitor(url1);
            monitor2 = monitorFactory.getMonitor(url2);
        }
        Assert.assertNotSame(monitor1, monitor2);
    }

}