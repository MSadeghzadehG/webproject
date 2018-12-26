
package com.alibaba.dubbo.common.json;

import java.io.IOException;


@Deprecated
public interface JSONConverter {
    
    void writeValue(Object obj, JSONWriter builder, boolean writeClass) throws IOException;

    
    Object readValue(Class<?> type, Object jv) throws IOException;
}