
package com.alibaba.dubbo.config.support;

import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.rpc.Exporter;
import com.alibaba.dubbo.rpc.Invocation;
import com.alibaba.dubbo.rpc.Invoker;
import com.alibaba.dubbo.rpc.Protocol;
import com.alibaba.dubbo.rpc.Result;
import com.alibaba.dubbo.rpc.RpcException;

public class MockProtocol implements Protocol {

    
    public int getDefaultPort() {

        return 0;
    }

    
    public <T> Exporter<T> export(Invoker<T> invoker) throws RpcException {
        return null;
    }

    
    public <T> Invoker<T> refer(Class<T> type, URL url) throws RpcException {

        final URL u = url;

        return new Invoker<T>() {
            public Class<T> getInterface() {
                return null;
            }

            public URL getUrl() {
                return u;
            }

            public boolean isAvailable() {
                return true;
            }

            public Result invoke(Invocation invocation) throws RpcException {
                return null;
            }

            public void destroy() {

            }
        };
    }

    
    public void destroy() {

    }

}