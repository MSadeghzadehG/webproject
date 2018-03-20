
package com.alibaba.dubbo.rpc.protocol.dubbo;


import com.alibaba.dubbo.common.Constants;
import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.common.extension.ExtensionLoader;
import com.alibaba.dubbo.rpc.Protocol;
import com.alibaba.dubbo.rpc.ProxyFactory;
import com.alibaba.dubbo.rpc.RpcException;
import com.alibaba.dubbo.rpc.protocol.dubbo.support.DemoService;
import com.alibaba.dubbo.rpc.protocol.dubbo.support.DemoServiceImpl;
import com.alibaba.dubbo.rpc.protocol.dubbo.support.NonSerialized;
import com.alibaba.dubbo.rpc.protocol.dubbo.support.RemoteService;
import com.alibaba.dubbo.rpc.protocol.dubbo.support.RemoteServiceImpl;
import com.alibaba.dubbo.rpc.protocol.dubbo.support.Type;
import com.alibaba.dubbo.rpc.service.EchoService;

import junit.framework.Assert;
import org.junit.Test;

import java.util.HashMap;
import java.util.Map;
import java.util.Set;

import static junit.framework.Assert.assertEquals;



public class DubboProtocolTest {
    private Protocol protocol = ExtensionLoader.getExtensionLoader(Protocol.class).getAdaptiveExtension();
    private ProxyFactory proxy = ExtensionLoader.getExtensionLoader(ProxyFactory.class).getAdaptiveExtension();

    @Test
    public void testDemoProtocol() throws Exception {
        DemoService service = new DemoServiceImpl();
        protocol.export(proxy.getInvoker(service, DemoService.class, URL.valueOf("dubbo:        service = proxy.getProxy(protocol.refer(DemoService.class, URL.valueOf("dubbo:        assertEquals(service.getSize(new String[]{"", "", ""}), 3);
    }

    @Test
    public void testDubboProtocol() throws Exception {
        DemoService service = new DemoServiceImpl();
        protocol.export(proxy.getInvoker(service, DemoService.class, URL.valueOf("dubbo:        service = proxy.getProxy(protocol.refer(DemoService.class, URL.valueOf("dubbo:        assertEquals(service.enumlength(new Type[]{}), Type.Lower);
        assertEquals(service.getSize(null), -1);
        assertEquals(service.getSize(new String[]{"", "", ""}), 3);
        Map<String, String> map = new HashMap<String, String>();
        map.put("aa", "bb");
        Set<String> set = service.keys(map);
        assertEquals(set.size(), 1);
        assertEquals(set.iterator().next(), "aa");
        service.invoke("dubbo:
        service = proxy.getProxy(protocol.refer(DemoService.class, URL.valueOf("dubbo:                StringBuffer buf = new StringBuffer();
        for (int i = 0; i < 1024 * 32 + 32; i++)
            buf.append('A');
        System.out.println(service.stringLength(buf.toString()));

                EchoService echo = proxy.getProxy(protocol.refer(EchoService.class, URL.valueOf("dubbo:        assertEquals(echo.$echo(buf.toString()), buf.toString());
        assertEquals(echo.$echo("test"), "test");
        assertEquals(echo.$echo("abcdefg"), "abcdefg");
        assertEquals(echo.$echo(1234), 1234);
    }

    @Test
    public void testDubboProtocolWithMina() throws Exception {
        DemoService service = new DemoServiceImpl();
        protocol.export(proxy.getInvoker(service, DemoService.class, URL.valueOf("dubbo:        service = proxy.getProxy(protocol.refer(DemoService.class, URL.valueOf("dubbo:        for (int i = 0; i < 10; i++) {
            assertEquals(service.enumlength(new Type[]{}), Type.Lower);
            assertEquals(service.getSize(null), -1);
            assertEquals(service.getSize(new String[]{"", "", ""}), 3);
        }
        Map<String, String> map = new HashMap<String, String>();
        map.put("aa", "bb");
        for (int i = 0; i < 10; i++) {
            Set<String> set = service.keys(map);
            assertEquals(set.size(), 1);
            assertEquals(set.iterator().next(), "aa");
            service.invoke("dubbo:        }

        service = proxy.getProxy(protocol.refer(DemoService.class, URL.valueOf("dubbo:                StringBuffer buf = new StringBuffer();
        for (int i = 0; i < 1024 * 32 + 32; i++)
            buf.append('A');
        System.out.println(service.stringLength(buf.toString()));

                EchoService echo = proxy.getProxy(protocol.refer(EchoService.class, URL.valueOf("dubbo:        for (int i = 0; i < 10; i++) {
            assertEquals(echo.$echo(buf.toString()), buf.toString());
            assertEquals(echo.$echo("test"), "test");
            assertEquals(echo.$echo("abcdefg"), "abcdefg");
            assertEquals(echo.$echo(1234), 1234);
        }
    }

    @Test
    public void testDubboProtocolMultiService() throws Exception {
        DemoService service = new DemoServiceImpl();
        protocol.export(proxy.getInvoker(service, DemoService.class, URL.valueOf("dubbo:        service = proxy.getProxy(protocol.refer(DemoService.class, URL.valueOf("dubbo:
        RemoteService remote = new RemoteServiceImpl();
        protocol.export(proxy.getInvoker(remote, RemoteService.class, URL.valueOf("dubbo:        remote = proxy.getProxy(protocol.refer(RemoteService.class, URL.valueOf("dubbo:
        service.sayHello("world");

                assertEquals("world", service.echo("world"));
        assertEquals("hello world@" + RemoteServiceImpl.class.getName(), remote.sayHello("world"));

        EchoService serviceEcho = (EchoService) service;
        assertEquals(serviceEcho.$echo("test"), "test");

        EchoService remoteEecho = (EchoService) remote;
        assertEquals(remoteEecho.$echo("ok"), "ok");
    }

    @Test
    public void testPerm() throws Exception {
        DemoService service = new DemoServiceImpl();
        protocol.export(proxy.getInvoker(service, DemoService.class, URL.valueOf("dubbo:        service = proxy.getProxy(protocol.refer(DemoService.class, URL.valueOf("dubbo:        long start = System.currentTimeMillis();
        for (int i = 0; i < 1000; i++)
            service.getSize(new String[]{"", "", ""});
        System.out.println("take:" + (System.currentTimeMillis() - start));
    }

    @Test
    public void testNonSerializedParameter() throws Exception {
        DemoService service = new DemoServiceImpl();
        protocol.export(proxy.getInvoker(service, DemoService.class, URL.valueOf("dubbo:        service = proxy.getProxy(protocol.refer(DemoService.class, URL.valueOf("dubbo:        try {
            service.nonSerializedParameter(new NonSerialized());
            Assert.fail();
        } catch (RpcException e) {
            Assert.assertTrue(e.getMessage().contains("com.alibaba.dubbo.rpc.protocol.dubbo.support.NonSerialized must implement java.io.Serializable"));
        }
    }

    @Test
    public void testReturnNonSerialized() throws Exception {
        DemoService service = new DemoServiceImpl();
        protocol.export(proxy.getInvoker(service, DemoService.class, URL.valueOf("dubbo:        service = proxy.getProxy(protocol.refer(DemoService.class, URL.valueOf("dubbo:        try {
            service.returnNonSerialized();
            Assert.fail();
        } catch (RpcException e) {
            Assert.assertTrue(e.getMessage().contains("com.alibaba.dubbo.rpc.protocol.dubbo.support.NonSerialized must implement java.io.Serializable"));
        }
    }
}