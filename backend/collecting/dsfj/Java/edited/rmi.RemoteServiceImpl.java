
package com.alibaba.dubbo.rpc.protocol.rmi;

import com.alibaba.dubbo.rpc.RpcContext;

import java.rmi.RemoteException;

public class RemoteServiceImpl implements RemoteService {
    public String getThreadName() throws RemoteException {
        System.out.println("RpcContext.getContext().getRemoteHost()=" + RpcContext.getContext().getRemoteHost());
        return Thread.currentThread().getName();
    }

    public String sayHello(String name) throws RemoteException {
        return "hello " + name + "@" + RemoteServiceImpl.class.getName();
    }
}