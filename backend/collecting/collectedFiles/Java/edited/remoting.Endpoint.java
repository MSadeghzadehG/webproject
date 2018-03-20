
package com.alibaba.dubbo.remoting;

import com.alibaba.dubbo.common.URL;

import java.net.InetSocketAddress;


public interface Endpoint {

    
    URL getUrl();

    
    ChannelHandler getChannelHandler();

    
    InetSocketAddress getLocalAddress();

    
    void send(Object message) throws RemotingException;

    
    void send(Object message, boolean sent) throws RemotingException;

    
    void close();

    
    void close(int timeout);

    void startClose();

    
    boolean isClosed();

}