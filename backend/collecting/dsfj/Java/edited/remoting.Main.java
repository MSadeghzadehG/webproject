
package com.alibaba.dubbo.remoting;

import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.remoting.exchange.ExchangeChannel;
import com.alibaba.dubbo.remoting.exchange.Exchangers;
import com.alibaba.dubbo.remoting.exchange.ResponseFuture;
import com.alibaba.dubbo.remoting.exchange.support.Replier;
import com.alibaba.dubbo.remoting.exchange.support.ReplierDispatcher;

import java.io.Serializable;
import java.util.Random;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;



public class Main {
    public static void main(String[] args) throws Exception {
        startServer(9010);
        mutliThreadTest(10, 9010);
        dataPackageTest(9010);
    }

    private static void startServer(int port) throws Exception {
        ReplierDispatcher dispatcher = new ReplierDispatcher();
        dispatcher.addReplier(RpcMessage.class, new RpcMessageHandler());
        dispatcher.addReplier(Object.class, new Replier<Object>() {
            public Object reply(ExchangeChannel channel, Object msg) {
                for (int i = 0; i < 10000; i++)
                    System.currentTimeMillis();
                System.out.println("handle:" + msg + ";thread:" + Thread.currentThread().getName());
                return new StringMessage("hello world");
            }
        });
        Exchangers.bind(URL.valueOf("dubbo:    }

    static void dataPackageTest(int port) throws Exception {
        ExchangeChannel client = Exchangers.connect(URL.valueOf("dubbo:        Random random = new Random();
        for (int i = 5; i < 100; i++) {
            StringBuilder sb = new StringBuilder();
            for (int j = 0; j < i * 100; j++)
                sb.append("(" + random.nextLong() + ")");
            Main.Data d = new Main.Data();
            d.setData(sb.toString());
            client.request(d).get();
        }
        System.out.println("send finished.");
    }

    static void mutliThreadTest(int tc, final int port) throws Exception {
        Executor exec = Executors.newFixedThreadPool(tc);
        for (int i = 0; i < tc; i++)
            exec.execute(new Runnable() {
                public void run() {
                    try {
                        test(port);
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }
            });
    }

    private static void test(int port) throws Exception {
        ExchangeChannel client = Exchangers.connect(URL.valueOf("dubbo:        MockResult result = (MockResult) client.request(new RpcMessage(DemoService.class.getName(), "plus", new Class<?>[]{int.class, int.class}, new Object[]{55, 25})).get();
        System.out.println("55+25=" + result.getResult());

        for (int i = 0; i < 100; i++)
            client.request(new RpcMessage(DemoService.class.getName(), "sayHello", new Class<?>[]{String.class}, new Object[]{"qianlei" + i}));

        for (int i = 0; i < 100; i++)
            client.request(new Main.Data());

        System.out.println("=====test invoke=====");
        for (int i = 0; i < 100; i++) {
            ResponseFuture future = client.request(new Main.Data());
            System.out.println("invoke and get");
            System.out.println("invoke result:" + future.get());
        }
        System.out.println("=====the end=====");
    }

    static class Data implements Serializable {
        private static final long serialVersionUID = -4666580993978548778L;

        private String mData = "";

        public Data() {
        }

        public String getData() {
            return mData;
        }

        public void setData(String data) {
            mData = data;
        }
    }

    static class StringMessage implements Serializable {
        private static final long serialVersionUID = 7193122183120113947L;

        private String mText;

        StringMessage(String msg) {
            mText = msg;
        }

        public String toString() {
            return mText;
        }
    }
}