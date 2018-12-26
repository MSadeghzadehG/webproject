
package com.alibaba.dubbo.rpc.support;

import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.rpc.Invocation;
import com.alibaba.dubbo.rpc.Invoker;
import com.alibaba.dubbo.rpc.Result;
import com.alibaba.dubbo.rpc.RpcException;
import com.alibaba.dubbo.rpc.RpcResult;


public class MyInvoker<T> implements Invoker<T> {

    URL url;
    Class<T> type;
    boolean hasException = false;

    public MyInvoker(URL url) {
        this.url = url;
        type = (Class<T>) DemoService.class;
    }

    public MyInvoker(URL url, boolean hasException) {
        this.url = url;
        type = (Class<T>) DemoService.class;
        this.hasException = hasException;
    }

    public Class<T> getInterface() {
        return type;
    }

    public URL getUrl() {
        return url;
    }

    public boolean isAvailable() {
        return false;
    }

    public Result invoke(Invocation invocation) throws RpcException {
        RpcResult result = new RpcResult();
        if (hasException == false) {
            result.setValue("alibaba");
            return result;
        } else {
            result.setException(new RuntimeException("mocked exception"));
            return result;
        }

    }

    public void destroy() {
    }

}