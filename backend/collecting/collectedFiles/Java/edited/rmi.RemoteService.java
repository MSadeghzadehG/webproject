
package com.alibaba.dubbo.rpc.protocol.rmi;

import java.rmi.Remote;
import java.rmi.RemoteException;

public interface RemoteService extends Remote {
    String sayHello(String name) throws RemoteException;

    String getThreadName() throws RemoteException;
}