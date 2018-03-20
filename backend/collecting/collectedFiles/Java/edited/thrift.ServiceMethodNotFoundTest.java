
package com.alibaba.dubbo.rpc.protocol.thrift;

import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.rpc.gen.dubbo.$__DemoStub;
import com.alibaba.dubbo.rpc.gen.dubbo.Demo;
import com.alibaba.dubbo.rpc.protocol.thrift.ext.MultiServiceProcessor;

import org.apache.thrift.protocol.TBinaryProtocol;
import org.apache.thrift.server.TThreadPoolServer;
import org.apache.thrift.transport.TServerSocket;
import org.apache.thrift.transport.TServerTransport;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import java.lang.reflect.Field;
import java.util.Map;

public class ServiceMethodNotFoundTest extends AbstractTest {

    private URL url;

    protected void init() throws Exception {

        TServerTransport serverTransport = new TServerSocket(PORT);

        DubboDemoImpl impl = new DubboDemoImpl();

        $__DemoStub.Processor processor = new $__DemoStub.Processor(impl);

                Field field = processor.getClass().getSuperclass().getDeclaredField("processMap");

        field.setAccessible(true);

        Object obj = field.get(processor);

        if (obj instanceof Map) {
            ((Map) obj).remove("echoString");
        }
        
        TBinaryProtocol.Factory bFactory = new TBinaryProtocol.Factory();

        MultiServiceProcessor wrapper = new MultiServiceProcessor();
        wrapper.addProcessor(Demo.class, processor);

        server = new TThreadPoolServer(
                new TThreadPoolServer.Args(serverTransport)
                        .inputProtocolFactory(bFactory)
                        .outputProtocolFactory(bFactory)
                        .inputTransportFactory(getTransportFactory())
                        .outputTransportFactory(getTransportFactory())
                        .processor(wrapper));

        Thread startTread = new Thread() {

            @Override
            public void run() {

                server.serve();
            }

        };

        startTread.start();

        while (!server.isServing()) {
            Thread.sleep(100);
        }

    }

    @Before
    public void setUp() throws Exception {

        init();

        protocol = new ThriftProtocol();

        url = URL.valueOf(ThriftProtocol.NAME + ":
    }

    @After
    public void tearDown() throws Exception {

        destroy();

        if (protocol != null) {
            protocol.destroy();
            protocol = null;
        }

        if (invoker != null) {
            invoker.destroy();
            invoker = null;
        }

    }

    @Test
    public void testServiceMethodNotFound() throws Exception {
                

    }

}
