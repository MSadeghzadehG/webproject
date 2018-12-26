
package com.alibaba.dubbo.rpc.protocol.dubbo;


import com.alibaba.dubbo.common.Constants;
import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.common.extension.ExtensionLoader;
import com.alibaba.dubbo.remoting.exchange.ExchangeClient;
import com.alibaba.dubbo.rpc.ProxyFactory;
import com.alibaba.dubbo.rpc.protocol.dubbo.support.ProtocolUtils;

import junit.framework.Assert;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import java.lang.reflect.Field;

import static org.junit.Assert.fail;


public class DubboInvokerAvilableTest {
    private static DubboProtocol protocol = DubboProtocol.getDubboProtocol();
    private static ProxyFactory proxy = ExtensionLoader.getExtensionLoader(ProxyFactory.class).getAdaptiveExtension();

    @BeforeClass
    public static void setUpBeforeClass() throws Exception {
    }

    @Before
    public void setUp() throws Exception {
    }

    @Test
    public void test_Normal_available() {
        URL url = URL.valueOf("dubbo:        ProtocolUtils.export(new DemoServiceImpl(), IDemoService.class, url);

        DubboInvoker<?> invoker = (DubboInvoker<?>) protocol.refer(IDemoService.class, url);
        Assert.assertEquals(true, invoker.isAvailable());
        invoker.destroy();
        Assert.assertEquals(false, invoker.isAvailable());
    }

    @Test
    public void test_Normal_ChannelReadOnly() throws Exception {
        URL url = URL.valueOf("dubbo:        ProtocolUtils.export(new DemoServiceImpl(), IDemoService.class, url);

        DubboInvoker<?> invoker = (DubboInvoker<?>) protocol.refer(IDemoService.class, url);
        Assert.assertEquals(true, invoker.isAvailable());

        getClients(invoker)[0].setAttribute(Constants.CHANNEL_ATTRIBUTE_READONLY_KEY, Boolean.TRUE);

        Assert.assertEquals(false, invoker.isAvailable());

                getClients(invoker)[0].removeAttribute(Constants.CHANNEL_ATTRIBUTE_READONLY_KEY);
    }

    @Test
    public void test_NoInvokers() throws Exception {
        URL url = URL.valueOf("dubbo:        ProtocolUtils.export(new DemoServiceImpl(), IDemoService.class, url);

        DubboInvoker<?> invoker = (DubboInvoker<?>) protocol.refer(IDemoService.class, url);

        ExchangeClient[] clients = getClients(invoker);
        clients[0].close();
        Assert.assertEquals(false, invoker.isAvailable());

    }

    @Test
    public void test_Lazy_ChannelReadOnly() throws Exception {
        URL url = URL.valueOf("dubbo:        ProtocolUtils.export(new DemoServiceImpl(), IDemoService.class, url);

        DubboInvoker<?> invoker = (DubboInvoker<?>) protocol.refer(IDemoService.class, url);
        Assert.assertEquals(true, invoker.isAvailable());

        try {
            getClients(invoker)[0].setAttribute(Constants.CHANNEL_ATTRIBUTE_READONLY_KEY, Boolean.TRUE);
            fail();
        } catch (IllegalStateException e) {

        }
        
        IDemoService service = (IDemoService) proxy.getProxy(invoker);
        Assert.assertEquals("ok", service.get());

        Assert.assertEquals(true, invoker.isAvailable());
        getClients(invoker)[0].setAttribute(Constants.CHANNEL_ATTRIBUTE_READONLY_KEY, Boolean.TRUE);
        Assert.assertEquals(false, invoker.isAvailable());
    }

    private ExchangeClient[] getClients(DubboInvoker<?> invoker) throws Exception {
        Field field = DubboInvoker.class.getDeclaredField("clients");
        field.setAccessible(true);
        ExchangeClient[] clients = (ExchangeClient[]) field.get(invoker);
        Assert.assertEquals(1, clients.length);
        return clients;
    }

    public interface IDemoService {
        public String get();
    }

    public class DemoServiceImpl implements IDemoService {
        public String get() {
            return "ok";
        }
    }
}