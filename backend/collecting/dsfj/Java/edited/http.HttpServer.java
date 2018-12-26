
package com.alibaba.dubbo.remoting.http;

import com.alibaba.dubbo.common.Resetable;
import com.alibaba.dubbo.common.URL;

import java.net.InetSocketAddress;

public interface HttpServer extends Resetable {

    
    HttpHandler getHttpHandler();

    
    URL getUrl();

    
    InetSocketAddress getLocalAddress();

    
    void close();

    
    void close(int timeout);

    
    boolean isBound();

    
    boolean isClosed();

}