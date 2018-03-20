
package com.alibaba.dubbo.remoting;

import com.alibaba.dubbo.common.bytecode.NoSuchMethodException;
import com.alibaba.dubbo.common.bytecode.Wrapper;
import com.alibaba.dubbo.remoting.exchange.ExchangeChannel;
import com.alibaba.dubbo.remoting.exchange.support.Replier;

import java.lang.reflect.InvocationTargetException;



public class RpcMessageHandler implements Replier<RpcMessage> {
    private final static ServiceProvider DEFAULT_PROVIDER = new ServiceProvider() {
        public Object getImplementation(String service) {
            String impl = service + "Impl";
            try {
                Class<?> cl = Thread.currentThread().getContextClassLoader().loadClass(impl);
                return cl.newInstance();
            } catch (Exception e) {
                e.printStackTrace();
            }
            return null;
        }
    };
    private ServiceProvider mProvider;

    public RpcMessageHandler() {
        this(DEFAULT_PROVIDER);
    }

    public RpcMessageHandler(ServiceProvider prov) {
        mProvider = prov;
    }

    public Class<RpcMessage> interest() {
        return RpcMessage.class;
    }

    public Object reply(ExchangeChannel channel, RpcMessage msg) throws RemotingException {
        String desc = msg.getMethodDesc();
        Object[] args = msg.getArguments();
        Object impl = mProvider.getImplementation(msg.getClassName());
        Wrapper wrap = Wrapper.getWrapper(impl.getClass());
        try {
            return new MockResult(wrap.invokeMethod(impl, desc, msg.getParameterTypes(), args));
        } catch (NoSuchMethodException e) {
            throw new RemotingException(channel, "Service method not found.");
        } catch (InvocationTargetException e) {
            return new MockResult(e.getTargetException());
        }

    }

    public static interface ServiceProvider {
        Object getImplementation(String service);
    }

}