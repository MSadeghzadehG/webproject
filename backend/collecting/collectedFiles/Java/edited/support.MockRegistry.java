
package com.alibaba.dubbo.config.support;

import com.alibaba.dubbo.common.Constants;
import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.registry.NotifyListener;
import com.alibaba.dubbo.registry.Registry;

import java.util.ArrayList;
import java.util.List;


public class MockRegistry implements Registry {

    static URL subscribedUrl = new URL("null", "0.0.0.0", 0);

    public static URL getSubscribedUrl() {
        return subscribedUrl;
    }

    
    public URL getUrl() {
        return null;
    }

    
    public boolean isAvailable() {
        return true;
    }

    
    public void destroy() {

    }

    
    public void register(URL url) {

    }

    
    public void unregister(URL url) {

    }

    
    public void subscribe(URL url, NotifyListener listener) {
        this.subscribedUrl = url;
        List<URL> urls = new ArrayList<URL>();

        urls.add(url.setProtocol("mockprotocol")
                .removeParameter(Constants.CATEGORY_KEY)
                .addParameter(Constants.METHODS_KEY, "sayHello"));

        listener.notify(urls);
    }

    
    public void unsubscribe(URL url, NotifyListener listener) {

    }

    
    public List<URL> lookup(URL url) {
        return null;
    }

}