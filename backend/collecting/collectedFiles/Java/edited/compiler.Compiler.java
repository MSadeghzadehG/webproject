
package com.alibaba.dubbo.common.compiler;


import com.alibaba.dubbo.common.extension.SPI;


@SPI("javassist")
public interface Compiler {

    
    Class<?> compile(String code, ClassLoader classLoader);

}
