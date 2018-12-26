
package com.alibaba.dubbo.registry.dubbo;

import com.alibaba.dubbo.common.Constants;
import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.common.Version;
import com.alibaba.dubbo.common.logger.Logger;
import com.alibaba.dubbo.common.logger.LoggerFactory;
import com.alibaba.dubbo.common.utils.NamedThreadFactory;
import com.alibaba.dubbo.common.utils.NetUtils;
import com.alibaba.dubbo.registry.NotifyListener;
import com.alibaba.dubbo.registry.RegistryService;
import com.alibaba.dubbo.registry.support.FailbackRegistry;
import com.alibaba.dubbo.rpc.Invoker;

import java.util.List;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ScheduledFuture;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.ReentrantLock;


public class DubboRegistry extends FailbackRegistry {

    private final static Logger logger = LoggerFactory.getLogger(DubboRegistry.class);

        private static final int RECONNECT_PERIOD_DEFAULT = 3 * 1000;

        private final ScheduledExecutorService scheduledExecutorService = Executors.newScheduledThreadPool(1, new NamedThreadFactory("DubboRegistryReconnectTimer", true));

        private final ScheduledFuture<?> reconnectFuture;

        private final ReentrantLock clientLock = new ReentrantLock();

    private final Invoker<RegistryService> registryInvoker;

    private final RegistryService registryService;

    public DubboRegistry(Invoker<RegistryService> registryInvoker, RegistryService registryService) {
        super(registryInvoker.getUrl());
        this.registryInvoker = registryInvoker;
        this.registryService = registryService;
                int reconnectPeriod = registryInvoker.getUrl().getParameter(Constants.REGISTRY_RECONNECT_PERIOD_KEY, RECONNECT_PERIOD_DEFAULT);
        reconnectFuture = scheduledExecutorService.scheduleWithFixedDelay(new Runnable() {
            public void run() {
                                try {
                    connect();
                } catch (Throwable t) {                     logger.error("Unexpected error occur at reconnect, cause: " + t.getMessage(), t);
                }
            }
        }, reconnectPeriod, reconnectPeriod, TimeUnit.MILLISECONDS);
    }

    protected final void connect() {
        try {
                        if (isAvailable()) {
                return;
            }
            if (logger.isInfoEnabled()) {
                logger.info("Reconnect to registry " + getUrl());
            }
            clientLock.lock();
            try {
                                if (isAvailable()) {
                    return;
                }
                recover();
            } finally {
                clientLock.unlock();
            }
        } catch (Throwable t) {             if (getUrl().getParameter(Constants.CHECK_KEY, true)) {
                if (t instanceof RuntimeException) {
                    throw (RuntimeException) t;
                }
                throw new RuntimeException(t.getMessage(), t);
            }
            logger.error("Failed to connect to registry " + getUrl().getAddress() + " from provider/consumer " + NetUtils.getLocalHost() + " use dubbo " + Version.getVersion() + ", cause: " + t.getMessage(), t);
        }
    }

    public boolean isAvailable() {
        if (registryInvoker == null)
            return false;
        return registryInvoker.isAvailable();
    }

    public void destroy() {
        super.destroy();
        try {
                        if (!reconnectFuture.isCancelled()) {
                reconnectFuture.cancel(true);
            }
        } catch (Throwable t) {
            logger.warn("Failed to cancel reconnect timer", t);
        }
        registryInvoker.destroy();
    }

    protected void doRegister(URL url) {
        registryService.register(url);
    }

    protected void doUnregister(URL url) {
        registryService.unregister(url);
    }

    protected void doSubscribe(URL url, NotifyListener listener) {
        registryService.subscribe(url, listener);
    }

    protected void doUnsubscribe(URL url, NotifyListener listener) {
        registryService.unsubscribe(url, listener);
    }

    public List<URL> lookup(URL url) {
        return registryService.lookup(url);
    }

}