
package com.alibaba.dubbo.rpc;

import com.alibaba.dubbo.common.Constants;
import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.common.utils.NetUtils;

import java.net.InetSocketAddress;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Future;
import java.util.concurrent.FutureTask;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;


public class RpcContext {

    private static final ThreadLocal<RpcContext> LOCAL = new ThreadLocal<RpcContext>() {
        @Override
        protected RpcContext initialValue() {
            return new RpcContext();
        }
    };
    private final Map<String, String> attachments = new HashMap<String, String>();
    private final Map<String, Object> values = new HashMap<String, Object>();
    private Future<?> future;

    private List<URL> urls;

    private URL url;

    private String methodName;

    private Class<?>[] parameterTypes;

    private Object[] arguments;

    private InetSocketAddress localAddress;

    private InetSocketAddress remoteAddress;
    @Deprecated
    private List<Invoker<?>> invokers;
    @Deprecated
    private Invoker<?> invoker;
    @Deprecated
    private Invocation invocation;

            private Object request;
    private Object response;

    protected RpcContext() {
    }

    
    public static RpcContext getContext() {
        return LOCAL.get();
    }

    
    public static void removeContext() {
        LOCAL.remove();
    }

    
    public Object getRequest() {
        return request;
    }

    
    @SuppressWarnings("unchecked")
    public <T> T getRequest(Class<T> clazz) {
        return (request != null && clazz.isAssignableFrom(request.getClass())) ? (T) request : null;
    }


    public void setRequest(Object request) {
        this.request = request;
    }

    
    public Object getResponse() {
        return response;
    }

    
    @SuppressWarnings("unchecked")
    public <T> T getResponse(Class<T> clazz) {
        return (response != null && clazz.isAssignableFrom(response.getClass())) ? (T) response : null;
    }

    public void setResponse(Object response) {
        this.response = response;
    }

    
    public boolean isProviderSide() {
        URL url = getUrl();
        if (url == null) {
            return false;
        }
        InetSocketAddress address = getRemoteAddress();
        if (address == null) {
            return false;
        }
        String host;
        if (address.getAddress() == null) {
            host = address.getHostName();
        } else {
            host = address.getAddress().getHostAddress();
        }
        return url.getPort() != address.getPort() ||
                !NetUtils.filterLocalHost(url.getIp()).equals(NetUtils.filterLocalHost(host));
    }

    
    public boolean isConsumerSide() {
        URL url = getUrl();
        if (url == null) {
            return false;
        }
        InetSocketAddress address = getRemoteAddress();
        if (address == null) {
            return false;
        }
        String host;
        if (address.getAddress() == null) {
            host = address.getHostName();
        } else {
            host = address.getAddress().getHostAddress();
        }
        return url.getPort() == address.getPort() &&
                NetUtils.filterLocalHost(url.getIp()).equals(NetUtils.filterLocalHost(host));
    }

    
    @SuppressWarnings("unchecked")
    public <T> Future<T> getFuture() {
        return (Future<T>) future;
    }

    
    public void setFuture(Future<?> future) {
        this.future = future;
    }

    public List<URL> getUrls() {
        return urls == null && url != null ? (List<URL>) Arrays.asList(url) : urls;
    }

    public void setUrls(List<URL> urls) {
        this.urls = urls;
    }

    public URL getUrl() {
        return url;
    }

    public void setUrl(URL url) {
        this.url = url;
    }

    
    public String getMethodName() {
        return methodName;
    }

    public void setMethodName(String methodName) {
        this.methodName = methodName;
    }

    
    public Class<?>[] getParameterTypes() {
        return parameterTypes;
    }

    public void setParameterTypes(Class<?>[] parameterTypes) {
        this.parameterTypes = parameterTypes;
    }

    
    public Object[] getArguments() {
        return arguments;
    }

    public void setArguments(Object[] arguments) {
        this.arguments = arguments;
    }

    
    public RpcContext setLocalAddress(String host, int port) {
        if (port < 0) {
            port = 0;
        }
        this.localAddress = InetSocketAddress.createUnresolved(host, port);
        return this;
    }

    
    public InetSocketAddress getLocalAddress() {
        return localAddress;
    }

    
    public RpcContext setLocalAddress(InetSocketAddress address) {
        this.localAddress = address;
        return this;
    }

    public String getLocalAddressString() {
        return getLocalHost() + ":" + getLocalPort();
    }

    
    public String getLocalHostName() {
        String host = localAddress == null ? null : localAddress.getHostName();
        if (host == null || host.length() == 0) {
            return getLocalHost();
        }
        return host;
    }

    
    public RpcContext setRemoteAddress(String host, int port) {
        if (port < 0) {
            port = 0;
        }
        this.remoteAddress = InetSocketAddress.createUnresolved(host, port);
        return this;
    }

    
    public InetSocketAddress getRemoteAddress() {
        return remoteAddress;
    }

    
    public RpcContext setRemoteAddress(InetSocketAddress address) {
        this.remoteAddress = address;
        return this;
    }

    
    public String getRemoteAddressString() {
        return getRemoteHost() + ":" + getRemotePort();
    }

    
    public String getRemoteHostName() {
        return remoteAddress == null ? null : remoteAddress.getHostName();
    }

    
    public String getLocalHost() {
        String host = localAddress == null ? null :
                localAddress.getAddress() == null ? localAddress.getHostName()
                        : NetUtils.filterLocalHost(localAddress.getAddress().getHostAddress());
        if (host == null || host.length() == 0) {
            return NetUtils.getLocalHost();
        }
        return host;
    }

    
    public int getLocalPort() {
        return localAddress == null ? 0 : localAddress.getPort();
    }

    
    public String getRemoteHost() {
        return remoteAddress == null ? null :
                remoteAddress.getAddress() == null ? remoteAddress.getHostName()
                        : NetUtils.filterLocalHost(remoteAddress.getAddress().getHostAddress());
    }

    
    public int getRemotePort() {
        return remoteAddress == null ? 0 : remoteAddress.getPort();
    }

    
    public String getAttachment(String key) {
        return attachments.get(key);
    }

    
    public RpcContext setAttachment(String key, String value) {
        if (value == null) {
            attachments.remove(key);
        } else {
            attachments.put(key, value);
        }
        return this;
    }

    
    public RpcContext removeAttachment(String key) {
        attachments.remove(key);
        return this;
    }

    
    public Map<String, String> getAttachments() {
        return attachments;
    }

    
    public RpcContext setAttachments(Map<String, String> attachment) {
        this.attachments.clear();
        if (attachment != null && attachment.size() > 0) {
            this.attachments.putAll(attachment);
        }
        return this;
    }

    public void clearAttachments() {
        this.attachments.clear();
    }

    
    public Map<String, Object> get() {
        return values;
    }

    
    public RpcContext set(String key, Object value) {
        if (value == null) {
            values.remove(key);
        } else {
            values.put(key, value);
        }
        return this;
    }

    
    public RpcContext remove(String key) {
        values.remove(key);
        return this;
    }

    
    public Object get(String key) {
        return values.get(key);
    }

    
    @Deprecated
    public boolean isServerSide() {
        return isProviderSide();
    }

    
    @Deprecated
    public boolean isClientSide() {
        return isConsumerSide();
    }

    
    @Deprecated
    @SuppressWarnings({"unchecked", "rawtypes"})
    public List<Invoker<?>> getInvokers() {
        return invokers == null && invoker != null ? (List) Arrays.asList(invoker) : invokers;
    }

    public RpcContext setInvokers(List<Invoker<?>> invokers) {
        this.invokers = invokers;
        if (invokers != null && !invokers.isEmpty()) {
            List<URL> urls = new ArrayList<URL>(invokers.size());
            for (Invoker<?> invoker : invokers) {
                urls.add(invoker.getUrl());
            }
            setUrls(urls);
        }
        return this;
    }

    
    @Deprecated
    public Invoker<?> getInvoker() {
        return invoker;
    }

    public RpcContext setInvoker(Invoker<?> invoker) {
        this.invoker = invoker;
        if (invoker != null) {
            setUrl(invoker.getUrl());
        }
        return this;
    }

    
    @Deprecated
    public Invocation getInvocation() {
        return invocation;
    }

    public RpcContext setInvocation(Invocation invocation) {
        this.invocation = invocation;
        if (invocation != null) {
            setMethodName(invocation.getMethodName());
            setParameterTypes(invocation.getParameterTypes());
            setArguments(invocation.getArguments());
        }
        return this;
    }

    
    @SuppressWarnings("unchecked")
    public <T> Future<T> asyncCall(Callable<T> callable) {
        try {
            try {
                setAttachment(Constants.ASYNC_KEY, Boolean.TRUE.toString());
                final T o = callable.call();
                                if (o != null) {
                    FutureTask<T> f = new FutureTask<T>(new Callable<T>() {
                        public T call() throws Exception {
                            return o;
                        }
                    });
                    f.run();
                    return f;
                } else {

                }
            } catch (Exception e) {
                throw new RpcException(e);
            } finally {
                removeAttachment(Constants.ASYNC_KEY);
            }
        } catch (final RpcException e) {
            return new Future<T>() {
                public boolean cancel(boolean mayInterruptIfRunning) {
                    return false;
                }

                public boolean isCancelled() {
                    return false;
                }

                public boolean isDone() {
                    return true;
                }

                public T get() throws InterruptedException, ExecutionException {
                    throw new ExecutionException(e.getCause());
                }

                public T get(long timeout, TimeUnit unit)
                        throws InterruptedException, ExecutionException,
                        TimeoutException {
                    return get();
                }
            };
        }
        return ((Future<T>) getContext().getFuture());
    }

    
    public void asyncCall(Runnable runnable) {
        try {
            setAttachment(Constants.RETURN_KEY, Boolean.FALSE.toString());
            runnable.run();
        } catch (Throwable e) {
                        throw new RpcException("oneway call error ." + e.getMessage(), e);
        } finally {
            removeAttachment(Constants.RETURN_KEY);
        }
    }
}