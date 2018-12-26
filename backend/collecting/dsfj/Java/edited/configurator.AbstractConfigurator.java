
package com.alibaba.dubbo.rpc.cluster.configurator;

import com.alibaba.dubbo.common.Constants;
import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.common.utils.NetUtils;
import com.alibaba.dubbo.rpc.cluster.Configurator;

import java.util.HashSet;
import java.util.Map;
import java.util.Set;


public abstract class AbstractConfigurator implements Configurator {

    private final URL configuratorUrl;

    public AbstractConfigurator(URL url) {
        if (url == null) {
            throw new IllegalArgumentException("configurator url == null");
        }
        this.configuratorUrl = url;
    }

    public static void main(String[] args) {
        System.out.println(URL.encode("timeout=100"));
    }

    public URL getUrl() {
        return configuratorUrl;
    }

    public URL configure(URL url) {
        if (configuratorUrl == null || configuratorUrl.getHost() == null
                || url == null || url.getHost() == null) {
            return url;
        }
                if (configuratorUrl.getPort() != 0) {
            if (url.getPort() == configuratorUrl.getPort()) {
                return configureIfMatch(url.getHost(), url);
            }
        } else {                                    if (url.getParameter(Constants.SIDE_KEY, Constants.PROVIDER).equals(Constants.CONSUMER)) {
                return configureIfMatch(NetUtils.getLocalHost(), url);            } else if (url.getParameter(Constants.SIDE_KEY, Constants.CONSUMER).equals(Constants.PROVIDER)) {
                return configureIfMatch(Constants.ANYHOST_VALUE, url);            }
        }
        return url;
    }

    private URL configureIfMatch(String host, URL url) {
        if (Constants.ANYHOST_VALUE.equals(configuratorUrl.getHost()) || host.equals(configuratorUrl.getHost())) {
            String configApplication = configuratorUrl.getParameter(Constants.APPLICATION_KEY,
                    configuratorUrl.getUsername());
            String currentApplication = url.getParameter(Constants.APPLICATION_KEY, url.getUsername());
            if (configApplication == null || Constants.ANY_VALUE.equals(configApplication)
                    || configApplication.equals(currentApplication)) {
                Set<String> condtionKeys = new HashSet<String>();
                condtionKeys.add(Constants.CATEGORY_KEY);
                condtionKeys.add(Constants.CHECK_KEY);
                condtionKeys.add(Constants.DYNAMIC_KEY);
                condtionKeys.add(Constants.ENABLED_KEY);
                for (Map.Entry<String, String> entry : configuratorUrl.getParameters().entrySet()) {
                    String key = entry.getKey();
                    String value = entry.getValue();
                    if (key.startsWith("~") || Constants.APPLICATION_KEY.equals(key) || Constants.SIDE_KEY.equals(key)) {
                        condtionKeys.add(key);
                        if (value != null && !Constants.ANY_VALUE.equals(value)
                                && !value.equals(url.getParameter(key.startsWith("~") ? key.substring(1) : key))) {
                            return url;
                        }
                    }
                }
                return doConfigure(url, configuratorUrl.removeParameters(condtionKeys));
            }
        }
        return url;
    }

    
    public int compareTo(Configurator o) {
        if (o == null) {
            return -1;
        }

        int ipCompare = getUrl().getHost().compareTo(o.getUrl().getHost());
        if (ipCompare == 0) {            int i = getUrl().getParameter(Constants.PRIORITY_KEY, 0),
                    j = o.getUrl().getParameter(Constants.PRIORITY_KEY, 0);
            if (i < j) {
                return -1;
            } else if (i > j) {
                return 1;
            } else {
                return 0;
            }
        } else {
            return ipCompare;
        }


    }

    protected abstract URL doConfigure(URL currentUrl, URL configUrl);

}
