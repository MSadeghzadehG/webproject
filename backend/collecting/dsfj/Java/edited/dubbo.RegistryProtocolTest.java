
package com.alibaba.dubbo.registry.dubbo;

import com.alibaba.dubbo.common.Constants;
import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.common.extension.ExtensionLoader;
import com.alibaba.dubbo.registry.NotifyListener;
import com.alibaba.dubbo.registry.RegistryFactory;
import com.alibaba.dubbo.registry.integration.RegistryProtocol;
import com.alibaba.dubbo.registry.support.AbstractRegistry;
import com.alibaba.dubbo.remoting.exchange.ExchangeClient;
import com.alibaba.dubbo.rpc.Exporter;
import com.alibaba.dubbo.rpc.Invocation;
import com.alibaba.dubbo.rpc.Invoker;
import com.alibaba.dubbo.rpc.Protocol;
import com.alibaba.dubbo.rpc.Result;
import com.alibaba.dubbo.rpc.cluster.support.FailfastCluster;
import com.alibaba.dubbo.rpc.protocol.AbstractInvoker;
import com.alibaba.dubbo.rpc.protocol.dubbo.DubboInvoker;
import com.alibaba.dubbo.rpc.protocol.dubbo.DubboProtocol;

import org.junit.Assert;
import org.junit.Test;

import java.util.ArrayList;
import java.util.List;

import static org.junit.Assert.assertEquals;


public class RegistryProtocolTest {

    static {
        SimpleRegistryExporter.exportIfAbsent(9090);
    }

    final String service = "com.alibaba.dubbo.registry.protocol.DemoService:1.0.0";
    final String serviceUrl = "dubbo:    final URL registryUrl = URL.valueOf("registry:    final private Protocol protocol = ExtensionLoader.getExtensionLoader(Protocol.class).getAdaptiveExtension();

    @Test
    public void testDefaultPort() {
        RegistryProtocol registryProtocol = new RegistryProtocol();
        assertEquals(9090, registryProtocol.getDefaultPort());
    }

    @Test(expected = IllegalArgumentException.class)
    public void testExportUrlNull() {
        RegistryProtocol registryProtocol = new RegistryProtocol();
        registryProtocol.setCluster(new FailfastCluster());

        Protocol dubboProtocol = DubboProtocol.getDubboProtocol();
        registryProtocol.setProtocol(dubboProtocol);
        Invoker<DemoService> invoker = new DubboInvoker<DemoService>(DemoService.class,
                registryUrl, new ExchangeClient[]{new MockedClient("10.20.20.20", 2222, true)});
        registryProtocol.export(invoker);
    }

    @Test
    public void testExport() {
        RegistryProtocol registryProtocol = new RegistryProtocol();
        registryProtocol.setCluster(new FailfastCluster());
        registryProtocol.setRegistryFactory(ExtensionLoader.getExtensionLoader(RegistryFactory.class).getAdaptiveExtension());

        Protocol dubboProtocol = DubboProtocol.getDubboProtocol();
        registryProtocol.setProtocol(dubboProtocol);
        URL newRegistryUrl = registryUrl.addParameter(Constants.EXPORT_KEY, serviceUrl);
        DubboInvoker<DemoService> invoker = new DubboInvoker<DemoService>(DemoService.class,
                newRegistryUrl, new ExchangeClient[]{new MockedClient("10.20.20.20", 2222, true)});
        Exporter<DemoService> exporter = registryProtocol.export(invoker);
        Exporter<DemoService> exporter2 = registryProtocol.export(invoker);
                Assert.assertNotSame(exporter, exporter2);
        exporter.unexport();
        exporter2.unexport();

    }

    @Test
    public void testNotifyOverride() throws Exception {
        URL newRegistryUrl = registryUrl.addParameter(Constants.EXPORT_KEY, serviceUrl);
        Invoker<RegistryProtocolTest> invoker = new MockInvoker<RegistryProtocolTest>(RegistryProtocolTest.class, newRegistryUrl);
        Exporter<?> exporter = protocol.export(invoker);
        RegistryProtocol rprotocol = RegistryProtocol.getRegistryProtocol();
        NotifyListener listener = getListener(rprotocol);
        List<URL> urls = new ArrayList<URL>();
        urls.add(URL.valueOf("override:        urls.add(URL.valueOf("override:        urls.add(URL.valueOf("override:        listener.notify(urls);

        assertEquals(true, exporter.getInvoker().isAvailable());
        assertEquals("100", exporter.getInvoker().getUrl().getParameter("timeout"));
        assertEquals("y", exporter.getInvoker().getUrl().getParameter("x"));

        exporter.unexport();
        destroyRegistryProtocol();

    }


    
    @Test
    public void testNotifyOverride_notmatch() throws Exception {
        URL newRegistryUrl = registryUrl.addParameter(Constants.EXPORT_KEY, serviceUrl);
        Invoker<RegistryProtocolTest> invoker = new MockInvoker<RegistryProtocolTest>(RegistryProtocolTest.class, newRegistryUrl);
        Exporter<?> exporter = protocol.export(invoker);
        RegistryProtocol rprotocol = RegistryProtocol.getRegistryProtocol();
        NotifyListener listener = getListener(rprotocol);
        List<URL> urls = new ArrayList<URL>();
        urls.add(URL.valueOf("override:        listener.notify(urls);
        assertEquals(true, exporter.getInvoker().isAvailable());
        assertEquals(null, exporter.getInvoker().getUrl().getParameter("timeout"));
        exporter.unexport();
        destroyRegistryProtocol();
    }

    
    @Test
    public void testDestoryRegistry() {
        URL newRegistryUrl = registryUrl.addParameter(Constants.EXPORT_KEY, serviceUrl);
        Invoker<RegistryProtocolTest> invoker = new MockInvoker<RegistryProtocolTest>(RegistryProtocolTest.class, newRegistryUrl);
        Exporter<?> exporter = protocol.export(invoker);
        destroyRegistryProtocol();
        assertEquals(false, exporter.getInvoker().isAvailable());

    }

    private void destroyRegistryProtocol() {
        Protocol registry = RegistryProtocol.getRegistryProtocol();
        registry.destroy();
    }

    private NotifyListener getListener(RegistryProtocol protocol) throws Exception {
        return protocol.getOverrideListeners().values().iterator().next();
    }

    static class MockInvoker<T> extends AbstractInvoker<T> {
        public MockInvoker(Class<T> type, URL url) {
            super(type, url);
        }

        @Override
        protected Result doInvoke(Invocation invocation) throws Throwable {
                        return null;
        }
    }

    static class MockRegistry extends AbstractRegistry {

        public MockRegistry(URL url) {
            super(url);
        }

        public boolean isAvailable() {
            return true;
        }
    }

}