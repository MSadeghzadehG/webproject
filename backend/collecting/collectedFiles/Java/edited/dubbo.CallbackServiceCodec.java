
package com.alibaba.dubbo.rpc.protocol.dubbo;

import com.alibaba.dubbo.common.Constants;
import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.common.bytecode.Wrapper;
import com.alibaba.dubbo.common.extension.ExtensionLoader;
import com.alibaba.dubbo.common.logger.Logger;
import com.alibaba.dubbo.common.logger.LoggerFactory;
import com.alibaba.dubbo.common.utils.ConcurrentHashSet;
import com.alibaba.dubbo.common.utils.StringUtils;
import com.alibaba.dubbo.remoting.Channel;
import com.alibaba.dubbo.remoting.RemotingException;
import com.alibaba.dubbo.rpc.Exporter;
import com.alibaba.dubbo.rpc.Invocation;
import com.alibaba.dubbo.rpc.Invoker;
import com.alibaba.dubbo.rpc.ProxyFactory;
import com.alibaba.dubbo.rpc.RpcInvocation;

import java.io.IOException;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;


class CallbackServiceCodec {
    private static final Logger logger = LoggerFactory.getLogger(CallbackServiceCodec.class);

    private static final ProxyFactory proxyFactory = ExtensionLoader.getExtensionLoader(ProxyFactory.class).getAdaptiveExtension();
    private static final DubboProtocol protocol = DubboProtocol.getDubboProtocol();
    private static final byte CALLBACK_NONE = 0x0;
    private static final byte CALLBACK_CREATE = 0x1;
    private static final byte CALLBACK_DESTROY = 0x2;
    private static final String INV_ATT_CALLBACK_KEY = "sys_callback_arg-";

    private static byte isCallBack(URL url, String methodName, int argIndex) {
                byte isCallback = CALLBACK_NONE;
        if (url != null) {
            String callback = url.getParameter(methodName + "." + argIndex + ".callback");
            if (callback != null) {
                if (callback.equalsIgnoreCase("true")) {
                    isCallback = CALLBACK_CREATE;
                } else if (callback.equalsIgnoreCase("false")) {
                    isCallback = CALLBACK_DESTROY;
                }
            }
        }
        return isCallback;
    }

    
    @SuppressWarnings({"unchecked", "rawtypes"})
    private static String exportOrunexportCallbackService(Channel channel, URL url, Class clazz, Object inst, Boolean export) throws IOException {
        int instid = System.identityHashCode(inst);

        Map<String, String> params = new HashMap<String, String>(3);
                params.put(Constants.IS_SERVER_KEY, Boolean.FALSE.toString());
                params.put(Constants.IS_CALLBACK_SERVICE, Boolean.TRUE.toString());
        String group = url.getParameter(Constants.GROUP_KEY);
        if (group != null && group.length() > 0) {
            params.put(Constants.GROUP_KEY, group);
        }
                params.put(Constants.METHODS_KEY, StringUtils.join(Wrapper.getWrapper(clazz).getDeclaredMethodNames(), ","));

        Map<String, String> tmpmap = new HashMap<String, String>(url.getParameters());
        tmpmap.putAll(params);
        tmpmap.remove(Constants.VERSION_KEY);        tmpmap.put(Constants.INTERFACE_KEY, clazz.getName());
        URL exporturl = new URL(DubboProtocol.NAME, channel.getLocalAddress().getAddress().getHostAddress(), channel.getLocalAddress().getPort(), clazz.getName() + "." + instid, tmpmap);

                String cacheKey = getClientSideCallbackServiceCacheKey(instid);
        String countkey = getClientSideCountKey(clazz.getName());
        if (export) {
                        if (!channel.hasAttribute(cacheKey)) {
                if (!isInstancesOverLimit(channel, url, clazz.getName(), instid, false)) {
                    Invoker<?> invoker = proxyFactory.getInvoker(inst, clazz, exporturl);
                                        Exporter<?> exporter = protocol.export(invoker);
                                        channel.setAttribute(cacheKey, exporter);
                    logger.info("export a callback service :" + exporturl + ", on " + channel + ", url is: " + url);
                    increaseInstanceCount(channel, countkey);
                }
            }
        } else {
            if (channel.hasAttribute(cacheKey)) {
                Exporter<?> exporter = (Exporter<?>) channel.getAttribute(cacheKey);
                exporter.unexport();
                channel.removeAttribute(cacheKey);
                decreaseInstanceCount(channel, countkey);
            }
        }
        return String.valueOf(instid);
    }

    
    @SuppressWarnings("unchecked")
    private static Object referOrdestroyCallbackService(Channel channel, URL url, Class<?> clazz, Invocation inv, int instid, boolean isRefer) {
        Object proxy = null;
        String invokerCacheKey = getServerSideCallbackInvokerCacheKey(channel, clazz.getName(), instid);
        String proxyCacheKey = getServerSideCallbackServiceCacheKey(channel, clazz.getName(), instid);
        proxy = channel.getAttribute(proxyCacheKey);
        String countkey = getServerSideCountKey(channel, clazz.getName());
        if (isRefer) {
            if (proxy == null) {
                URL referurl = URL.valueOf("callback:                referurl = referurl.addParametersIfAbsent(url.getParameters()).removeParameter(Constants.METHODS_KEY);
                if (!isInstancesOverLimit(channel, referurl, clazz.getName(), instid, true)) {
                    @SuppressWarnings("rawtypes")
                    Invoker<?> invoker = new ChannelWrappedInvoker(clazz, channel, referurl, String.valueOf(instid));
                    proxy = proxyFactory.getProxy(invoker);
                    channel.setAttribute(proxyCacheKey, proxy);
                    channel.setAttribute(invokerCacheKey, invoker);
                    increaseInstanceCount(channel, countkey);

                                                            Set<Invoker<?>> callbackInvokers = (Set<Invoker<?>>) channel.getAttribute(Constants.CHANNEL_CALLBACK_KEY);
                    if (callbackInvokers == null) {
                        callbackInvokers = new ConcurrentHashSet<Invoker<?>>(1);
                        callbackInvokers.add(invoker);
                        channel.setAttribute(Constants.CHANNEL_CALLBACK_KEY, callbackInvokers);
                    }
                    logger.info("method " + inv.getMethodName() + " include a callback service :" + invoker.getUrl() + ", a proxy :" + invoker + " has been created.");
                }
            }
        } else {
            if (proxy != null) {
                Invoker<?> invoker = (Invoker<?>) channel.getAttribute(invokerCacheKey);
                try {
                    Set<Invoker<?>> callbackInvokers = (Set<Invoker<?>>) channel.getAttribute(Constants.CHANNEL_CALLBACK_KEY);
                    if (callbackInvokers != null) {
                        callbackInvokers.remove(invoker);
                    }
                    invoker.destroy();
                } catch (Exception e) {
                    logger.error(e.getMessage(), e);
                }
                                channel.removeAttribute(proxyCacheKey);
                channel.removeAttribute(invokerCacheKey);
                decreaseInstanceCount(channel, countkey);
            }
        }
        return proxy;
    }

    private static String getClientSideCallbackServiceCacheKey(int instid) {
        return Constants.CALLBACK_SERVICE_KEY + "." + instid;
    }

    private static String getServerSideCallbackServiceCacheKey(Channel channel, String interfaceClass, int instid) {
        return Constants.CALLBACK_SERVICE_PROXY_KEY + "." + System.identityHashCode(channel) + "." + interfaceClass + "." + instid;
    }

    private static String getServerSideCallbackInvokerCacheKey(Channel channel, String interfaceClass, int instid) {
        return getServerSideCallbackServiceCacheKey(channel, interfaceClass, instid) + "." + "invoker";
    }

    private static String getClientSideCountKey(String interfaceClass) {
        return Constants.CALLBACK_SERVICE_KEY + "." + interfaceClass + ".COUNT";
    }

    private static String getServerSideCountKey(Channel channel, String interfaceClass) {
        return Constants.CALLBACK_SERVICE_PROXY_KEY + "." + System.identityHashCode(channel) + "." + interfaceClass + ".COUNT";
    }

    private static boolean isInstancesOverLimit(Channel channel, URL url, String interfaceClass, int instid, boolean isServer) {
        Integer count = (Integer) channel.getAttribute(isServer ? getServerSideCountKey(channel, interfaceClass) : getClientSideCountKey(interfaceClass));
        int limit = url.getParameter(Constants.CALLBACK_INSTANCES_LIMIT_KEY, Constants.DEFAULT_CALLBACK_INSTANCES);
        if (count != null && count >= limit) {
                        throw new IllegalStateException("interface " + interfaceClass + " `s callback instances num exceed providers limit :" + limit
                    + " ,current num: " + (count + 1) + ". The new callback service will not work !!! you can cancle the callback service which exported before. channel :" + channel);
        } else {
            return false;
        }
    }

    private static void increaseInstanceCount(Channel channel, String countkey) {
        try {
                        Integer count = (Integer) channel.getAttribute(countkey);
            if (count == null) {
                count = 1;
            } else {
                count++;
            }
            channel.setAttribute(countkey, count);
        } catch (Exception e) {
            logger.error(e.getMessage(), e);
        }
    }

    private static void decreaseInstanceCount(Channel channel, String countkey) {
        try {
            Integer count = (Integer) channel.getAttribute(countkey);
            if (count == null || count <= 0) {
                return;
            } else {
                count--;
            }
            channel.setAttribute(countkey, count);
        } catch (Exception e) {
            logger.error(e.getMessage(), e);
        }
    }

    public static Object encodeInvocationArgument(Channel channel, RpcInvocation inv, int paraIndex) throws IOException {
                URL url = inv.getInvoker() == null ? null : inv.getInvoker().getUrl();
        byte callbackstatus = isCallBack(url, inv.getMethodName(), paraIndex);
        Object[] args = inv.getArguments();
        Class<?>[] pts = inv.getParameterTypes();
        switch (callbackstatus) {
            case CallbackServiceCodec.CALLBACK_NONE:
                return args[paraIndex];
            case CallbackServiceCodec.CALLBACK_CREATE:
                inv.setAttachment(INV_ATT_CALLBACK_KEY + paraIndex, exportOrunexportCallbackService(channel, url, pts[paraIndex], args[paraIndex], true));
                return null;
            case CallbackServiceCodec.CALLBACK_DESTROY:
                inv.setAttachment(INV_ATT_CALLBACK_KEY + paraIndex, exportOrunexportCallbackService(channel, url, pts[paraIndex], args[paraIndex], false));
                return null;
            default:
                return args[paraIndex];
        }
    }

    public static Object decodeInvocationArgument(Channel channel, RpcInvocation inv, Class<?>[] pts, int paraIndex, Object inObject) throws IOException {
                        URL url = null;
        try {
            url = DubboProtocol.getDubboProtocol().getInvoker(channel, inv).getUrl();
        } catch (RemotingException e) {
            if (logger.isInfoEnabled()) {
                logger.info(e.getMessage(), e);
            }
            return inObject;
        }
        byte callbackstatus = isCallBack(url, inv.getMethodName(), paraIndex);
        switch (callbackstatus) {
            case CallbackServiceCodec.CALLBACK_NONE:
                return inObject;
            case CallbackServiceCodec.CALLBACK_CREATE:
                try {
                    return referOrdestroyCallbackService(channel, url, pts[paraIndex], inv, Integer.parseInt(inv.getAttachment(INV_ATT_CALLBACK_KEY + paraIndex)), true);
                } catch (Exception e) {
                    logger.error(e.getMessage(), e);
                    throw new IOException(StringUtils.toString(e));
                }
            case CallbackServiceCodec.CALLBACK_DESTROY:
                try {
                    return referOrdestroyCallbackService(channel, url, pts[paraIndex], inv, Integer.parseInt(inv.getAttachment(INV_ATT_CALLBACK_KEY + paraIndex)), false);
                } catch (Exception e) {
                    throw new IOException(StringUtils.toString(e));
                }
            default:
                return inObject;
        }
    }
}