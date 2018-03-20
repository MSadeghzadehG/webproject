
package com.alibaba.dubbo.examples.async;

import com.alibaba.dubbo.examples.async.api.AsyncService;
import com.alibaba.dubbo.rpc.RpcContext;

import org.springframework.context.support.ClassPathXmlApplicationContext;

import java.util.concurrent.Callable;
import java.util.concurrent.Future;


public class AsyncConsumer {

    public static void main(String[] args) throws Exception {
        String config = AsyncConsumer.class.getPackage().getName().replace('.', '/') + "/async-consumer.xml";
        ClassPathXmlApplicationContext context = new ClassPathXmlApplicationContext(config);
        context.start();

        final AsyncService asyncService = (AsyncService) context.getBean("asyncService");

        Future<String> f = RpcContext.getContext().asyncCall(new Callable<String>() {
            public String call() throws Exception {
                return asyncService.sayHello("async call request");
            }
        });

        System.out.println("async call ret :" + f.get());

        RpcContext.getContext().asyncCall(new Runnable() {
            public void run() {
                asyncService.sayHello("oneway call request1");
                asyncService.sayHello("oneway call request2");
            }
        });

        System.in.read();
    }

}
