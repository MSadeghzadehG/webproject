
package com.alibaba.dubbo.common.logger;

import com.alibaba.dubbo.common.extension.SPI;

import java.io.File;


@SPI
public interface LoggerAdapter {

    
    Logger getLogger(Class<?> key);

    
    Logger getLogger(String key);

    
    Level getLevel();

    
    void setLevel(Level level);

    
    File getFile();

    
    void setFile(File file);
}