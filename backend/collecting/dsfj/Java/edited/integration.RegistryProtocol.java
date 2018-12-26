
package com.alibaba.dubbo.registry.integration;

import com.alibaba.dubbo.common.Constants;
import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.common.extension.ExtensionLoader;
import com.alibaba.dubbo.common.logger.Logger;
import com.alibaba.dubbo.common.logger.LoggerFactory;
import com.alibaba.dubbo.common.utils.ConfigUtils;
import com.alibaba.dubbo.common.utils.NamedThreadFactory;
import com.alibaba.dubbo.common.utils.StringUtils;
import com.alibaba.dubbo.common.utils.UrlUtils;
import com.alibaba.dubbo.registry.NotifyListener;
import com.alibaba.dubbo.registry.Registry;
import com.alibaba.dubbo.registry.RegistryFactory;
import com.alibaba.dubbo.registry.RegistryService;
import com.alibaba.dubbo.registry.support.ProviderConsumerRegTable;
import com.alibaba.dubbo.rpc.Exporter;
import com.alibaba.dubbo.rpc.Invoker;
import com.alibaba.dubbo.rpc.Protocol;
import com.alibaba.dubbo.rpc.ProxyFactory;
import com.alibaba.dubbo.rpc.RpcException;
import com.alibaba.dubbo.rpc.cluster.Cluster;
import com.alibaba.dubbo.rpc.cluster.Configurator;
import com.alibaba.dubbo.rpc.protocol.InvokerWrapper;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import static com.alibaba.dubbo.common.Constants.ACCEPT_FOREIGN_IP;
import static com.alibaba.dubbo.common.Constants.QOS_ENABLE;
import static com.alibaba.dubbo.common.Constants.QOS_PORT;


public class RegistryProtocol implements Protocol {

    private final static Logger logger = LoggerFactory.getLogger(RegistryProtocol.class);
    private static RegistryProtocol INSTANCE;
    private final Map<URL, NotifyListener> overrideListeners = new ConcurrentHashMap<URL, NotifyListener>();
            private final Map<String, ExporterChangeableWrapper<?>> bounds = new ConcurrentHashMap<String, ExporterChangeableWrapper<?>>();
    private Cluster cluster;
    private Protocol protocol;
    private RegistryFactory registryFactory;
    private ProxyFactory proxyFactory;

    public RegistryProtocol() {
        INSTANCE = this;
    }

    public static RegistryProtocol getRegistryProtocol() {
        if (INSTANCE == null) {
            ExtensionLoader.getExtensionLoader(Protocol.class).getExtension(Constants.REGISTRY_PROTOCOL);         }
        return INSTANCE;
    }

        private static String[] getFilteredKeys(URL url) {
        Map<String, String> params = url.getParameters();
        if (params != null && !params.isEmpty()) {
            List<String> filteredKeys = new ArrayList<String>();
            for (Map.Entry<String, String> entry : params.entrySet()) {
                if (entry != null && entry.getKey() != null && entry.getKey().startsWith(Constants.HIDE_KEY_PREFIX)) {
                    filteredKeys.add(entry.getKey());
                }
            }
            return filteredKeys.toArray(new String[filteredKeys.size()]);
        } else {
            return new String[]{};
        }
    }

    public void setCluster(Cluster cluster) {
        this.cluster = cluster;
    }

    public void setProtocol(Protocol protocol) {
        this.protocol = protocol;
    }

    public void setRegistryFactory(RegistryFactory registryFactory) {
        this.registryFactory = registryFactory;
    }

    public void setProxyFactory(ProxyFactory proxyFactory) {
        this.proxyFactory = proxyFactory;
    }

    public int getDefaultPort() {
        return 9090;
    }

    public Map<URL, NotifyListener> getOverrideListeners() {
        return overrideListeners;
    }

    public void register(URL registryUrl, URL registedProviderUrl) {
        Registry registry = registryFactory.getRegistry(registryUrl);
        registry.register(registedProviderUrl);
    }

    public <T> Exporter<T> export(final Invoker<T> originInvoker) throws RpcException {
                final ExporterChangeableWrapper<T> exporter = doLocalExport(originInvoker);

        URL registryUrl = getRegistryUrl(originInvoker);

                final Registry registry = getRegistry(originInvoker);
        final URL registedProviderUrl = getRegistedProviderUrl(originInvoker);

                boolean register = registedProviderUrl.getParameter("register", true);

        ProviderConsumerRegTable.registerProvider(originInvoker, registryUrl, registedProviderUrl);

        if (register) {
            register(registryUrl, registedProviderUrl);
            ProviderConsumerRegTable.getProviderWrapper(originInvoker).setReg(true);
        }

                        final URL overrideSubscribeUrl = getSubscribedOverrideUrl(registedProviderUrl);
        final OverrideListener overrideSubscribeListener = new OverrideListener(overrideSubscribeUrl, originInvoker);
        overrideListeners.put(overrideSubscribeUrl, overrideSubscribeListener);
        registry.subscribe(overrideSubscribeUrl, overrideSubscribeListener);
                return new DestroyableExporter<T>(exporter, originInvoker, overrideSubscribeUrl, registedProviderUrl);
    }

    @SuppressWarnings("unchecked")
    private <T> ExporterChangeableWrapper<T> doLocalExport(final Invoker<T> originInvoker) {
        String key = getCacheKey(originInvoker);
        ExporterChangeableWrapper<T> exporter = (ExporterChangeableWrapper<T>) bounds.get(key);
        if (exporter == null) {
            synchronized (bounds) {
                exporter = (ExporterChangeableWrapper<T>) bounds.get(key);
                if (exporter == null) {
                    final Invoker<?> invokerDelegete = new InvokerDelegete<T>(originInvoker, getProviderUrl(originInvoker));
                    exporter = new ExporterChangeableWrapper<T>((Exporter<T>) protocol.export(invokerDelegete), originInvoker);
                    bounds.put(key, exporter);
                }
            }
        }
        return exporter;
    }

    
    @SuppressWarnings("unchecked")
    private <T> void doChangeLocalExport(final Invoker<T> originInvoker, URL newInvokerUrl) {
        String key = getCacheKey(originInvoker);
        final ExporterChangeableWrapper<T> exporter = (ExporterChangeableWrapper<T>) bounds.get(key);
        if (exporter == null) {
            logger.warn(new IllegalStateException("error state, exporter should not be null"));
        } else {
            final Invoker<T> invokerDelegete = new InvokerDelegete<T>(originInvoker, newInvokerUrl);
            exporter.setExporter(protocol.export(invokerDelegete));
        }
    }

    
    private Registry getRegistry(final Invoker<?> originInvoker) {
        URL registryUrl = getRegistryUrl(originInvoker);
        return registryFactory.getRegistry(registryUrl);
    }

    private URL getRegistryUrl(Invoker<?> originInvoker) {
        URL registryUrl = originInvoker.getUrl();
        if (Constants.REGISTRY_PROTOCOL.equals(registryUrl.getProtocol())) {
            String protocol = registryUrl.getParameter(Constants.REGISTRY_KEY, Constants.DEFAULT_DIRECTORY);
            registryUrl = registryUrl.setProtocol(protocol).removeParameter(Constants.REGISTRY_KEY);
        }
        return registryUrl;
    }


    
    private URL getRegistedProviderUrl(final Invoker<?> originInvoker) {
        URL providerUrl = getProviderUrl(originInvoker);
                final URL registedProviderUrl = providerUrl.removeParameters(getFilteredKeys(providerUrl))
                .removeParameter(Constants.MONITOR_KEY)
                .removeParameter(Constants.BIND_IP_KEY)
                .removeParameter(Constants.BIND_PORT_KEY)
                .removeParameter(QOS_ENABLE)
                .removeParameter(QOS_PORT)
                .removeParameter(ACCEPT_FOREIGN_IP);
        return registedProviderUrl;
    }

    private URL getSubscribedOverrideUrl(URL registedProviderUrl) {
        return registedProviderUrl.setProtocol(Constants.PROVIDER_PROTOCOL)
                .addParameters(Constants.CATEGORY_KEY, Constants.CONFIGURATORS_CATEGORY,
                        Constants.CHECK_KEY, String.valueOf(false));
    }

    
    private URL getProviderUrl(final Invoker<?> origininvoker) {
        String export = origininvoker.getUrl().getParameterAndDecoded(Constants.EXPORT_KEY);
        if (export == null || export.length() == 0) {
            throw new IllegalArgumentException("The registry export url is null! registry: " + origininvoker.getUrl());
        }

        URL providerUrl = URL.valueOf(export);
        return providerUrl;
    }

    
    private String getCacheKey(final Invoker<?> originInvoker) {
        URL providerUrl = getProviderUrl(originInvoker);
        String key = providerUrl.removeParameters("dynamic", "enabled").toFullString();
        return key;
    }

    @SuppressWarnings("unchecked")
    public <T> Invoker<T> refer(Class<T> type, URL url) throws RpcException {
        url = url.setProtocol(url.getParameter(Constants.REGISTRY_KEY, Constants.DEFAULT_REGISTRY)).removeParameter(Constants.REGISTRY_KEY);
        Registry registry = registryFactory.getRegistry(url);
        if (RegistryService.class.equals(type)) {
            return proxyFactory.getInvoker((T) registry, type, url);
        }

                Map<String, String> qs = StringUtils.parseQueryString(url.getParameterAndDecoded(Constants.REFER_KEY));
        String group = qs.get(Constants.GROUP_KEY);
        if (group != null && group.length() > 0) {
            if ((Constants.COMMA_SPLIT_PATTERN.split(group)).length > 1
                    || "*".equals(group)) {
                return doRefer(getMergeableCluster(), registry, type, url);
            }
        }
        return doRefer(cluster, registry, type, url);
    }

    private Cluster getMergeableCluster() {
        return ExtensionLoader.getExtensionLoader(Cluster.class).getExtension("mergeable");
    }

    private <T> Invoker<T> doRefer(Cluster cluster, Registry registry, Class<T> type, URL url) {
        RegistryDirectory<T> directory = new RegistryDirectory<T>(type, url);
        directory.setRegistry(registry);
        directory.setProtocol(protocol);
                Map<String, String> parameters = new HashMap<String, String>(directory.getUrl().getParameters());
        URL subscribeUrl = new URL(Constants.CONSUMER_PROTOCOL, parameters.remove(Constants.REGISTER_IP_KEY), 0, type.getName(), parameters);
        if (!Constants.ANY_VALUE.equals(url.getServiceInterface())
                && url.getParameter(Constants.REGISTER_KEY, true)) {
            registry.register(subscribeUrl.addParameters(Constants.CATEGORY_KEY, Constants.CONSUMERS_CATEGORY,
                    Constants.CHECK_KEY, String.valueOf(false)));
        }
        directory.subscribe(subscribeUrl.addParameter(Constants.CATEGORY_KEY,
                Constants.PROVIDERS_CATEGORY
                        + "," + Constants.CONFIGURATORS_CATEGORY
                        + "," + Constants.ROUTERS_CATEGORY));

        Invoker invoker = cluster.join(directory);
        ProviderConsumerRegTable.registerConsumer(invoker, url, subscribeUrl, directory);
        return invoker;
    }

    public void destroy() {
        List<Exporter<?>> exporters = new ArrayList<Exporter<?>>(bounds.values());
        for (Exporter<?> exporter : exporters) {
            exporter.unexport();
        }
        bounds.clear();
    }

    public static class InvokerDelegete<T> extends InvokerWrapper<T> {
        private final Invoker<T> invoker;

        
        public InvokerDelegete(Invoker<T> invoker, URL url) {
            super(invoker, url);
            this.invoker = invoker;
        }

        public Invoker<T> getInvoker() {
            if (invoker instanceof InvokerDelegete) {
                return ((InvokerDelegete<T>) invoker).getInvoker();
            } else {
                return invoker;
            }
        }
    }

    
    private class OverrideListener implements NotifyListener {

        private final URL subscribeUrl;
        private final Invoker originInvoker;

        public OverrideListener(URL subscribeUrl, Invoker originalInvoker) {
            this.subscribeUrl = subscribeUrl;
            this.originInvoker = originalInvoker;
        }

        
        public synchronized void notify(List<URL> urls) {
            logger.debug("original override urls: " + urls);
            List<URL> matchedUrls = getMatchedUrls(urls, subscribeUrl);
            logger.debug("subscribe url: " + subscribeUrl + ", override urls: " + matchedUrls);
                        if (matchedUrls.isEmpty()) {
                return;
            }

            List<Configurator> configurators = RegistryDirectory.toConfigurators(matchedUrls);

            final Invoker<?> invoker;
            if (originInvoker instanceof InvokerDelegete) {
                invoker = ((InvokerDelegete<?>) originInvoker).getInvoker();
            } else {
                invoker = originInvoker;
            }
                        URL originUrl = RegistryProtocol.this.getProviderUrl(invoker);
            String key = getCacheKey(originInvoker);
            ExporterChangeableWrapper<?> exporter = bounds.get(key);
            if (exporter == null) {
                logger.warn(new IllegalStateException("error state, exporter should not be null"));
                return;
            }
                        URL currentUrl = exporter.getInvoker().getUrl();
                        URL newUrl = getConfigedInvokerUrl(configurators, originUrl);
            if (!currentUrl.equals(newUrl)) {
                RegistryProtocol.this.doChangeLocalExport(originInvoker, newUrl);
                logger.info("exported provider url changed, origin url: " + originUrl + ", old export url: " + currentUrl + ", new export url: " + newUrl);
            }
        }

        private List<URL> getMatchedUrls(List<URL> configuratorUrls, URL currentSubscribe) {
            List<URL> result = new ArrayList<URL>();
            for (URL url : configuratorUrls) {
                URL overrideUrl = url;
                                if (url.getParameter(Constants.CATEGORY_KEY) == null
                        && Constants.OVERRIDE_PROTOCOL.equals(url.getProtocol())) {
                    overrideUrl = url.addParameter(Constants.CATEGORY_KEY, Constants.CONFIGURATORS_CATEGORY);
                }

                                if (UrlUtils.isMatch(currentSubscribe, overrideUrl)) {
                    result.add(url);
                }
            }
            return result;
        }

                private URL getConfigedInvokerUrl(List<Configurator> configurators, URL url) {
            for (Configurator configurator : configurators) {
                url = configurator.configure(url);
            }
            return url;
        }
    }

    
    private class ExporterChangeableWrapper<T> implements Exporter<T> {

        private final Invoker<T> originInvoker;
        private Exporter<T> exporter;

        public ExporterChangeableWrapper(Exporter<T> exporter, Invoker<T> originInvoker) {
            this.exporter = exporter;
            this.originInvoker = originInvoker;
        }

        public Invoker<T> getOriginInvoker() {
            return originInvoker;
        }

        public Invoker<T> getInvoker() {
            return exporter.getInvoker();
        }

        public void setExporter(Exporter<T> exporter) {
            this.exporter = exporter;
        }

        public void unexport() {
            String key = getCacheKey(this.originInvoker);
            bounds.remove(key);
            exporter.unexport();
        }
    }

    static private class DestroyableExporter<T> implements Exporter<T> {

        public static final ExecutorService executor = Executors.newSingleThreadExecutor(new NamedThreadFactory("Exporter-Unexport", true));

        private Exporter<T> exporter;
        private Invoker<T> originInvoker;
        private URL subscribeUrl;
        private URL registerUrl;

        public DestroyableExporter(Exporter<T> exporter, Invoker<T> originInvoker, URL subscribeUrl, URL registerUrl) {
            this.exporter = exporter;
            this.originInvoker = originInvoker;
            this.subscribeUrl = subscribeUrl;
            this.registerUrl = registerUrl;
        }

        public Invoker<T> getInvoker() {
            return exporter.getInvoker();
        }

        public void unexport() {
            Registry registry = RegistryProtocol.INSTANCE.getRegistry(originInvoker);
            try {
                registry.unregister(registerUrl);
            } catch (Throwable t) {
                logger.warn(t.getMessage(), t);
            }
            try {
                NotifyListener listener = RegistryProtocol.INSTANCE.overrideListeners.remove(subscribeUrl);
                registry.unsubscribe(subscribeUrl, listener);
            } catch (Throwable t) {
                logger.warn(t.getMessage(), t);
            }

            executor.submit(new Runnable() {
                @Override
                public void run() {
                    try {
                        int timeout = ConfigUtils.getServerShutdownTimeout();
                        if (timeout > 0) {
                            logger.info("Waiting " + timeout + "ms for registry to notify all consumers before unexport. Usually, this is called when you use dubbo API");
                            Thread.sleep(timeout);
                        }
                        exporter.unexport();
                    } catch (Throwable t) {
                        logger.warn(t.getMessage(), t);
                    }
                }
            });
        }
    }
}
