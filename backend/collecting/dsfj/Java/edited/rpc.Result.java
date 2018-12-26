
package com.alibaba.dubbo.rpc;

import java.util.Map;


public interface Result {

    
    Object getValue();

    
    Throwable getException();

    
    boolean hasException();

    
    Object recreate() throws Throwable;

    
    @Deprecated
    Object getResult();


    
    Map<String, String> getAttachments();

    
    String getAttachment(String key);

    
    String getAttachment(String key, String defaultValue);

}