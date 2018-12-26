
package com.alibaba.dubbo.rpc.cluster.filter;


public class DemoServiceMock implements DemoService {
    public String sayHello(String name) {
        return name;
    }

    public int plus(int a, int b) {
        return a + b;
    }
}