

package com.alibaba.com.caucho.hessian.io;

import java.util.HashMap;


public class ExtSerializerFactory extends AbstractSerializerFactory {
    private HashMap _serializerMap = new HashMap();
    private HashMap _deserializerMap = new HashMap();

    
    public void addSerializer(Class cl, Serializer serializer) {
        _serializerMap.put(cl, serializer);
    }

    
    public void addDeserializer(Class cl, Deserializer deserializer) {
        _deserializerMap.put(cl, deserializer);
    }

    
    public Serializer getSerializer(Class cl)
            throws HessianProtocolException {
        return (Serializer) _serializerMap.get(cl);
    }

    
    public Deserializer getDeserializer(Class cl)
            throws HessianProtocolException {
        return (Deserializer) _deserializerMap.get(cl);
    }
}
