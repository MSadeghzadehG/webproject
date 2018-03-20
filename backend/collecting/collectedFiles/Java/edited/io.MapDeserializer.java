

package com.alibaba.com.caucho.hessian.io;

import java.io.IOException;
import java.lang.reflect.Constructor;
import java.lang.reflect.Type;
import java.util.HashMap;
import java.util.Map;
import java.util.SortedMap;
import java.util.TreeMap;


public class MapDeserializer extends AbstractMapDeserializer {
    private Class _type;
    private Constructor _ctor;

    public MapDeserializer(Class type) {
        if (type == null)
            type = HashMap.class;

        _type = type;

        Constructor[] ctors = type.getConstructors();
        for (int i = 0; i < ctors.length; i++) {
            if (ctors[i].getParameterTypes().length == 0)
                _ctor = ctors[i];
        }

        if (_ctor == null) {
            try {
                _ctor = HashMap.class.getConstructor(new Class[0]);
            } catch (Exception e) {
                throw new IllegalStateException(e);
            }
        }
    }

    public Class getType() {
        if (_type != null)
            return _type;
        else
            return HashMap.class;
    }

    public Object readMap(AbstractHessianInput in)
            throws IOException {
        return readMap(in, null, null);
    }

    
    @Override
    public Object readMap(AbstractHessianInput in, Class<?> expectKeyType, Class<?> expectValueType) throws IOException {
        Map map;

        if (_type == null)
            map = new HashMap();
        else if (_type.equals(Map.class))
            map = new HashMap();
        else if (_type.equals(SortedMap.class))
            map = new TreeMap();
        else {
            try {
                map = (Map) _ctor.newInstance();
            } catch (Exception e) {
                throw new IOExceptionWrapper(e);
            }
        }

        in.addRef(map);

        doReadMap(in, map, expectKeyType, expectValueType);

        in.readEnd();

        return map;
    }

    protected void doReadMap(AbstractHessianInput in, Map map, Class<?> keyType, Class<?> valueType) throws IOException {
        Deserializer keyDeserializer = null, valueDeserializer = null;

        SerializerFactory factory = findSerializerFactory(in);
        if(keyType != null){
            keyDeserializer = factory.getDeserializer(keyType.getName());
        }
        if(valueType != null){
            valueDeserializer = factory.getDeserializer(valueType.getName());
        }

        while (!in.isEnd()) {
            map.put(keyDeserializer != null ? keyDeserializer.readObject(in) : in.readObject(),
                    valueDeserializer != null? valueDeserializer.readObject(in) : in.readObject());
        }
    }

    @Override
    public Object readObject(AbstractHessianInput in,
                             String[] fieldNames)
            throws IOException {
        Map map = createMap();

        int ref = in.addRef(map);

        for (int i = 0; i < fieldNames.length; i++) {
            String name = fieldNames[i];

            map.put(name, in.readObject());
        }

        return map;
    }

    private Map createMap()
            throws IOException {

        if (_type == null)
            return new HashMap();
        else if (_type.equals(Map.class))
            return new HashMap();
        else if (_type.equals(SortedMap.class))
            return new TreeMap();
        else {
            try {
                return (Map) _ctor.newInstance();
            } catch (Exception e) {
                throw new IOExceptionWrapper(e);
            }
        }
    }
}
