
package com.alibaba.dubbo.rpc;

import java.util.Map;


public interface Invocation {

    
    String getMethodName();

    
    Class<?>[] getParameterTypes();

    
    Object[] getArguments();

    
    Map<String, String> getAttachments();

    
    String getAttachment(String key);

    
    String getAttachment(String key, String defaultValue);

    
    Invoker<?> getInvoker();

}