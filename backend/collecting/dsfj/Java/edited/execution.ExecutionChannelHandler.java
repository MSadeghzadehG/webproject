
package com.alibaba.dubbo.remoting.transport.dispatcher.execution;

import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.remoting.Channel;
import com.alibaba.dubbo.remoting.ChannelHandler;
import com.alibaba.dubbo.remoting.ExecutionException;
import com.alibaba.dubbo.remoting.RemotingException;
import com.alibaba.dubbo.remoting.exchange.Request;
import com.alibaba.dubbo.remoting.exchange.Response;
import com.alibaba.dubbo.remoting.transport.dispatcher.ChannelEventRunnable;
import com.alibaba.dubbo.remoting.transport.dispatcher.ChannelEventRunnable.ChannelState;
import com.alibaba.dubbo.remoting.transport.dispatcher.WrappedChannelHandler;

import java.util.concurrent.RejectedExecutionException;

public class ExecutionChannelHandler extends WrappedChannelHandler {

    public ExecutionChannelHandler(ChannelHandler handler, URL url) {
        super(handler, url);
    }

    public void connected(Channel channel) throws RemotingException {
        executor.execute(new ChannelEventRunnable(channel, handler, ChannelState.CONNECTED));
    }

    public void disconnected(Channel channel) throws RemotingException {
        executor.execute(new ChannelEventRunnable(channel, handler, ChannelState.DISCONNECTED));
    }

    public void received(Channel channel, Object message) throws RemotingException {
    	try {
            executor.execute(new ChannelEventRunnable(channel, handler, ChannelState.RECEIVED, message));
        } catch (Throwable t) {
                                	if(message instanceof Request &&
        			t instanceof RejectedExecutionException){
        		Request request = (Request)message;
        		if(request.isTwoWay()){
        			String msg = "Server side("+url.getIp()+","+url.getPort()+") threadpool is exhausted ,detail msg:"+t.getMessage();
        			Response response = new Response(request.getId(), request.getVersion());
        			response.setStatus(Response.SERVER_THREADPOOL_EXHAUSTED_ERROR);
        			response.setErrorMessage(msg);
        			channel.send(response);
        			return;
        		}
        	}
            throw new ExecutionException(message, channel, getClass() + " error when process received event .", t);
        }
    }

    public void caught(Channel channel, Throwable exception) throws RemotingException {
        executor.execute(new ChannelEventRunnable(channel, handler, ChannelState.CAUGHT, exception));
    }

}