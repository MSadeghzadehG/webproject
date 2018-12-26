
package com.alibaba.dubbo.remoting.exchange;

import com.alibaba.dubbo.remoting.Server;

import java.net.InetSocketAddress;
import java.util.Collection;


public interface ExchangeServer extends Server {

    
    Collection<ExchangeChannel> getExchangeChannels();

    
    ExchangeChannel getExchangeChannel(InetSocketAddress remoteAddress);

}