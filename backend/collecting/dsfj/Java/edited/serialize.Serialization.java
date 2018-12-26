
package com.alibaba.dubbo.common.serialize;

import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.common.extension.Adaptive;
import com.alibaba.dubbo.common.extension.SPI;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;


@SPI("hessian2")
public interface Serialization {

    
    byte getContentTypeId();

    
    String getContentType();

    
    @Adaptive
    ObjectOutput serialize(URL url, OutputStream output) throws IOException;

    
    @Adaptive
    ObjectInput deserialize(URL url, InputStream input) throws IOException;

}