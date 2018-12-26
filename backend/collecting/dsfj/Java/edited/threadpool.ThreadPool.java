
package com.alibaba.dubbo.common.threadpool;

import com.alibaba.dubbo.common.Constants;
import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.common.extension.Adaptive;
import com.alibaba.dubbo.common.extension.SPI;

import java.util.concurrent.Executor;


@SPI("fixed")
public interface ThreadPool {

    
    @Adaptive({Constants.THREADPOOL_KEY})
    Executor getExecutor(URL url);

}