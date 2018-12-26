
package com.alibaba.dubbo.remoting;

import java.io.Serializable;



public class MockResult implements Serializable {
    private static final long serialVersionUID = -3630485157441794463L;

    private final Object mResult;

    public MockResult(Object result) {
        mResult = result;
    }

    public Object getResult() {
        return mResult;
    }
}