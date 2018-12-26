

package com.alibaba.dubbo.common.store;

import com.alibaba.dubbo.common.extension.SPI;

import java.util.Map;

@SPI("simple")
public interface DataStore {

    
    Map<String, Object> get(String componentName);

    Object get(String componentName, String key);

    void put(String componentName, String key, Object value);

    void remove(String componentName, String key);

}
