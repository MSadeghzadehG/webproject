
package com.alibaba.dubbo.common.extensionloader.ext4;

import com.alibaba.dubbo.common.extension.Adaptive;
import com.alibaba.dubbo.common.extension.SPI;

import java.util.List;

@SPI("impl1")
public interface NoUrlParamExt {
        @Adaptive
    String bark(String name, List<Object> list);
}