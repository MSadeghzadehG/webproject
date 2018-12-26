
package com.alibaba.dubbo.examples.heartbeat;

import com.alibaba.dubbo.remoting.Channel;
import com.alibaba.dubbo.remoting.RemotingException;
import com.alibaba.dubbo.remoting.exchange.ExchangeHandler;
import com.alibaba.dubbo.remoting.exchange.Request;
import com.alibaba.dubbo.remoting.exchange.Response;
import com.alibaba.dubbo.remoting.exchange.support.header.HeaderExchangeHandler;

import java.util.concurrent.atomic.AtomicInteger;

public class HeartBeatExchangeHandler extends HeaderExchangeHandler {

    private AtomicInteger heartBeatCounter = new AtomicInteger(0);

    public HeartBeatExchangeHandler(ExchangeHandler handler) {
        super(handler);
    }

    @Override
    public void received(Channel channel, Object message) throws RemotingException {
        if (message instanceof Request) {
            Request req = (Request) message;
            if (req.isHeartbeat()) {
                heartBeatCounter.incrementAndGet();
                channel.setAttribute(KEY_READ_TIMESTAMP, System.currentTimeMillis());
                Response res = new Response(req.getId(), req.getVersion());
                res.setEvent(req.getData() == null ? null : req.getData().toString());
                channel.send(res);
            }
        } else {
            super.received(channel, message);
        }
    }

    public int getHeartBeatCount() {
        return heartBeatCounter.get();
    }

}
