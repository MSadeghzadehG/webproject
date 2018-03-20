
package com.alibaba.dubbo.rpc.benchmark;

import java.util.concurrent.CountDownLatch;
import java.util.concurrent.CyclicBarrier;


public class RpcClient extends AbstractClientRunnable {
    private static String message = null;
    private static int length = 100;

    static {
        length = Integer.valueOf(System.getProperty("message.length", "1000"));
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < length; i++) {
            sb.append("a");
        }
        message = sb.toString();
    }

    
    public RpcClient(String targetIP, int targetPort, int clientNums, int rpcTimeout, CyclicBarrier barrier,
                     CountDownLatch latch, long startTime, long endTime) {
        super(targetIP, targetPort, clientNums, rpcTimeout, barrier, latch, startTime, endTime);
    }

    @SuppressWarnings({"unchecked", "rawtypes"})
    @Override
    public Object invoke(ServiceFactory serviceFactory) {
        DemoService demoService = (DemoService) serviceFactory.get(DemoService.class);
        Object result = demoService.sendRequest(message);
        return result;
       
    }
}
