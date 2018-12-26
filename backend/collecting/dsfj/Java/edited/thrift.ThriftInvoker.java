
package com.alibaba.dubbo.rpc.protocol.thrift;

import com.alibaba.dubbo.common.Constants;
import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.common.utils.AtomicPositiveInteger;
import com.alibaba.dubbo.remoting.RemotingException;
import com.alibaba.dubbo.remoting.TimeoutException;
import com.alibaba.dubbo.remoting.exchange.ExchangeClient;
import com.alibaba.dubbo.rpc.Invocation;
import com.alibaba.dubbo.rpc.Invoker;
import com.alibaba.dubbo.rpc.Result;
import com.alibaba.dubbo.rpc.RpcContext;
import com.alibaba.dubbo.rpc.RpcException;
import com.alibaba.dubbo.rpc.RpcInvocation;
import com.alibaba.dubbo.rpc.protocol.AbstractInvoker;

import java.util.Set;
import java.util.concurrent.locks.ReentrantLock;

public class ThriftInvoker<T> extends AbstractInvoker<T> {

    private final ExchangeClient[] clients;

    private final AtomicPositiveInteger index = new AtomicPositiveInteger();

    private final ReentrantLock destroyLock = new ReentrantLock();

    private final Set<Invoker<?>> invokers;

    public ThriftInvoker(Class<T> service, URL url, ExchangeClient[] clients) {
        this(service, url, clients, null);
    }

    public ThriftInvoker(Class<T> type, URL url, ExchangeClient[] clients, Set<Invoker<?>> invokers) {
        super(type, url,
                new String[]{Constants.INTERFACE_KEY, Constants.GROUP_KEY,
                        Constants.TOKEN_KEY, Constants.TIMEOUT_KEY});
        this.clients = clients;
        this.invokers = invokers;
    }

    @Override
    protected Result doInvoke(Invocation invocation) throws Throwable {

        RpcInvocation inv = (RpcInvocation) invocation;

        final String methodName;

        methodName = invocation.getMethodName();

        inv.setAttachment(Constants.PATH_KEY, getUrl().getPath());

                inv.setAttachment(ThriftCodec.PARAMETER_CLASS_NAME_GENERATOR, getUrl().getParameter(
                ThriftCodec.PARAMETER_CLASS_NAME_GENERATOR, DubboClassNameGenerator.NAME));

        ExchangeClient currentClient;

        if (clients.length == 1) {
            currentClient = clients[0];
        } else {
            currentClient = clients[index.getAndIncrement() % clients.length];
        }

        try {
            int timeout = getUrl().getMethodParameter(
                    methodName, Constants.TIMEOUT_KEY, Constants.DEFAULT_TIMEOUT);

            RpcContext.getContext().setFuture(null);

            return (Result) currentClient.request(inv, timeout).get();

        } catch (TimeoutException e) {
            throw new RpcException(RpcException.TIMEOUT_EXCEPTION, e.getMessage(), e);
        } catch (RemotingException e) {
            throw new RpcException(RpcException.NETWORK_EXCEPTION, e.getMessage(), e);
        }

    }

    @Override
    public boolean isAvailable() {

        if (!super.isAvailable()) {
            return false;
        }

        for (ExchangeClient client : clients) {
            if (client.isConnected()
                    && !client.hasAttribute(Constants.CHANNEL_ATTRIBUTE_READONLY_KEY)) {
                                return true;
            }
        }
        return false;
    }

    public void destroy() {
                                if (super.isDestroyed()) {
            return;
        } else {
                        destroyLock.lock();

            try {

                if (super.isDestroyed()) {
                    return;
                }

                super.destroy();

                if (invokers != null) {
                    invokers.remove(this);
                }

                for (ExchangeClient client : clients) {

                    try {
                        client.close();
                    } catch (Throwable t) {
                        logger.warn(t.getMessage(), t);
                    }

                }

            } finally {
                destroyLock.unlock();
            }

        }

    }

}
