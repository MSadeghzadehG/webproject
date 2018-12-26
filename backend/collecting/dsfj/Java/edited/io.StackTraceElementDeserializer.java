

package com.alibaba.com.caucho.hessian.io;


public class StackTraceElementDeserializer extends JavaDeserializer {
    public StackTraceElementDeserializer() {
        super(StackTraceElement.class);
    }

    @Override
    protected Object instantiate()
            throws Exception {
        return new StackTraceElement("", "", "", 0);
    }
}
