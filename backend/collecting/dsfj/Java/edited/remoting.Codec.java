
package com.alibaba.dubbo.remoting;

import com.alibaba.dubbo.common.Constants;
import com.alibaba.dubbo.common.extension.Adaptive;
import com.alibaba.dubbo.common.extension.SPI;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;


@Deprecated
@SPI
public interface Codec {

    
    Object NEED_MORE_INPUT = new Object();

    
    @Adaptive({Constants.CODEC_KEY})
    void encode(Channel channel, OutputStream output, Object message) throws IOException;

    
    @Adaptive({Constants.CODEC_KEY})
    Object decode(Channel channel, InputStream input) throws IOException;

}