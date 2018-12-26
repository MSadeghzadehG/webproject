
package com.alibaba.dubbo.rpc.protocol.dubbo.support;


import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.common.extension.ExtensionLoader;
import com.alibaba.dubbo.common.utils.NetUtils;
import com.alibaba.dubbo.rpc.Invoker;
import com.alibaba.dubbo.rpc.Protocol;
import com.alibaba.dubbo.rpc.ProxyFactory;
import com.alibaba.dubbo.rpc.service.GenericService;

import org.junit.Assert;
import org.junit.Ignore;
import org.junit.Test;

import java.util.HashMap;
import java.util.Map;

public class EnumBak {

    private Protocol protocol = ExtensionLoader.getExtensionLoader(Protocol.class).getAdaptiveExtension();
    private ProxyFactory proxy = ExtensionLoader.getExtensionLoader(ProxyFactory.class).getAdaptiveExtension();

    @Test
    public void testNormal() {
        int port = NetUtils.getAvailablePort();
        URL serviceurl = URL.valueOf("dubbo:                + "&interface=" + DemoService.class.getName()
                + "&timeout=" + Integer.MAX_VALUE
        );
        DemoService demo = new DemoServiceImpl();
        Invoker<DemoService> invoker = proxy.getInvoker(demo, DemoService.class, serviceurl);
        protocol.export(invoker);

        URL consumerurl = serviceurl;
        Invoker<DemoService> reference = protocol.refer(DemoService.class, consumerurl);
        DemoService demoProxy = (DemoService) proxy.getProxy(reference);
        Assert.assertEquals((byte) -128, demoProxy.getbyte((byte) -128));

        reference.destroy();
    }

    @Ignore
    @Test
    public void testExportService() throws InterruptedException {
        int port = NetUtils.getAvailablePort();
        URL serviceurl = URL.valueOf("dubbo:        );
        DemoService demo = new DemoServiceImpl();
        Invoker<DemoService> invoker = proxy.getInvoker(demo, DemoService.class, serviceurl);
        protocol.export(invoker);
        synchronized (EnumBak.class) {
            EnumBak.class.wait();
        }

    }

    @Test
    public void testNormalEnum() {
        int port = NetUtils.getAvailablePort();
        URL serviceurl = URL.valueOf("dubbo:        );
        DemoService demo = new DemoServiceImpl();
        Invoker<DemoService> invoker = proxy.getInvoker(demo, DemoService.class, serviceurl);
        protocol.export(invoker);

        URL consumerurl = serviceurl;
        Invoker<DemoService> reference = protocol.refer(DemoService.class, consumerurl);
        DemoService demoProxy = (DemoService) proxy.getProxy(reference);
        Type type = demoProxy.enumlength(Type.High);
        System.out.println(type);
        Assert.assertEquals(Type.High, type);

        invoker.destroy();
        reference.destroy();
    }

        @Ignore
    @Test
    public void testEnumCompat() {
        int port = 20880;
        URL consumerurl = URL.valueOf("dubbo:        );
        Invoker<DemoService> reference = protocol.refer(DemoService.class, consumerurl);
        DemoService demoProxy = (DemoService) proxy.getProxy(reference);
        Type type = demoProxy.enumlength(Type.High);
        System.out.println(type);
        Assert.assertEquals(Type.High, type);
        reference.destroy();
    }

        @Ignore
    @Test
    public void testGenricEnumCompat() {
        int port = 20880;
        URL consumerurl = URL.valueOf("dubbo:        );
        Invoker<GenericService> reference = protocol.refer(GenericService.class, consumerurl);

        GenericService demoProxy = (GenericService) proxy.getProxy(reference);
        Object obj = demoProxy.$invoke("enumlength", new String[]{Type[].class.getName()}, new Object[]{new Type[]{Type.High, Type.High}});
        System.out.println("obj---------->" + obj);
        reference.destroy();
    }

        @Ignore
    @Test
    public void testGenricCustomArg() {

        int port = 20880;
        URL consumerurl = URL.valueOf("dubbo:        );
        Invoker<GenericService> reference = protocol.refer(GenericService.class, consumerurl);

        GenericService demoProxy = (GenericService) proxy.getProxy(reference);
        Map<String, Object> arg = new HashMap<String, Object>();
        arg.put("type", "High");
        arg.put("name", "hi");

        Object obj = demoProxy.$invoke("get", new String[]{"com.alibaba.dubbo.rpc.CustomArgument"}, new Object[]{arg});
        System.out.println("obj---------->" + obj);
        reference.destroy();
    }

    @Ignore
    @Test
    public void testGenericExport() throws InterruptedException {
        int port = NetUtils.getAvailablePort();
        port = 20880;
        URL serviceurl = URL.valueOf("dubbo:        );
        DemoService demo = new DemoServiceImpl();
        Invoker<DemoService> invoker = proxy.getInvoker(demo, DemoService.class, serviceurl);
        protocol.export(invoker);


                Thread.sleep(Integer.MAX_VALUE);
    }

    @Test
    public void testGenericEnum() throws InterruptedException {
        int port = NetUtils.getAvailablePort();
        URL serviceurl = URL.valueOf("dubbo:        );
        DemoService demo = new DemoServiceImpl();
        Invoker<DemoService> invoker = proxy.getInvoker(demo, DemoService.class, serviceurl);
        protocol.export(invoker);

        URL consumerurl = serviceurl;

        Invoker<GenericService> reference = protocol.refer(GenericService.class, consumerurl);

        GenericService demoProxy = (GenericService) proxy.getProxy(reference);
        Object obj = demoProxy.$invoke("enumlength", new String[]{Type[].class.getName()}, new Object[]{new Type[]{Type.High, Type.High}});
        System.out.println("obj---------->" + obj);

        invoker.destroy();
        reference.destroy();
    }
}
