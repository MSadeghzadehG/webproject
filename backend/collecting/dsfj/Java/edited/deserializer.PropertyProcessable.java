package com.alibaba.fastjson.parser.deserializer;

import java.lang.reflect.Type;


public interface PropertyProcessable extends ParseProcess {
    
    Type getType(String name);

    
    void apply(String name, Object value);
}
