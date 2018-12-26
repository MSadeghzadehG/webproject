
package com.alibaba.dubbo.remoting;

import com.alibaba.dubbo.common.Resetable;

import java.net.InetSocketAddress;
import java.util.Collection;


public interface Server extends Endpoint, Resetable {

    
    boolean isBound();

    
    Collection<Channel> getChannels();

    
    Channel getChannel(InetSocketAddress remoteAddress);

    @Deprecated
    void reset(com.alibaba.dubbo.common.Parameters parameters);

}