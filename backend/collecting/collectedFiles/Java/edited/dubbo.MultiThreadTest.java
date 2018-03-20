
package com.alibaba.dubbo.rpc.protocol.dubbo;

import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.common.extension.ExtensionLoader;
import com.alibaba.dubbo.rpc.Exporter;
import com.alibaba.dubbo.rpc.Protocol;
import com.alibaba.dubbo.rpc.ProxyFactory;
import com.alibaba.dubbo.rpc.protocol.dubbo.support.DemoService;
import com.alibaba.dubbo.rpc.protocol.dubbo.support.DemoServiceImpl;

import junit.framework.TestCase;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;

public class MultiThreadTest extends TestCase {

    private Protocol protocol = ExtensionLoader.getExtensionLoader(Protocol.class).getAdaptiveExtension();
    private ProxyFactory proxy = ExtensionLoader.getExtensionLoader(ProxyFactory.class).getAdaptiveExtension();

    public void testDubboMultiThreadInvoke() throws Exception {
        Exporter<?> rpcExporter = protocol.export(proxy.getInvoker(new DemoServiceImpl(), DemoService.class, URL.valueOf("dubbo:
        final AtomicInteger counter = new AtomicInteger();
        final DemoService service = proxy.getProxy(protocol.refer(DemoService.class, URL.valueOf("dubbo:        assertEquals(service.getSize(new String[]{"123", "456", "789"}), 3);

        final StringBuffer sb = new StringBuffer();
        for (int i = 0; i < 1024 * 64 + 32; i++)
            sb.append('A');
        assertEquals(sb.toString(), service.echo(sb.toString()));

        ExecutorService exec = Executors.newFixedThreadPool(10);
        for (int i = 0; i < 10; i++) {
            final int fi = i;
            exec.execute(new Runnable() {
                public void run() {
                    for (int i = 0; i < 30; i++) {
                        System.out.println(fi + ":" + counter.getAndIncrement());
                        assertEquals(service.echo(sb.toString()), sb.toString());
                    }
                }
            });
        }
        exec.shutdown();
        exec.awaitTermination(10, TimeUnit.SECONDS);
        rpcExporter.unexport();
    }
}