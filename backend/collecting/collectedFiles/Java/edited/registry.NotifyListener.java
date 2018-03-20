
package com.alibaba.dubbo.registry;

import com.alibaba.dubbo.common.URL;

import java.util.List;


public interface NotifyListener {

    
    void notify(List<URL> urls);

}