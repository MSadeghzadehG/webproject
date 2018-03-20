
package com.alibaba.dubbo.rpc;

import java.io.Serializable;



class DemoRequest implements Serializable {
    private static final long serialVersionUID = -2579095288792344869L;

    private String mServiceName;

    private String mMethodName;

    private Class<?>[] mParameterTypes;

    private Object[] mArguments;

    public DemoRequest(String serviceName, String methodName, Class<?>[] parameterTypes, Object[] args) {
        mServiceName = serviceName;
        mMethodName = methodName;
        mParameterTypes = parameterTypes;
        mArguments = args;
    }

    public String getServiceName() {
        return mServiceName;
    }

    public String getMethodName() {
        return mMethodName;
    }

    public Class<?>[] getParameterTypes() {
        return mParameterTypes;
    }

    public Object[] getArguments() {
        return mArguments;
    }
}