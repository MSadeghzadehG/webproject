
package com.alibaba.fastjson.serializer;

import java.io.IOException;
import java.lang.reflect.Type;


public interface JSONSerializable {

    
    void write(JSONSerializer serializer,                Object fieldName,                Type fieldType,                int features     ) throws IOException;
}
