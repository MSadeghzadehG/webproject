

package com.alibaba.com.caucho.hessian.io;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.SortedSet;
import java.util.TreeSet;


public class CollectionDeserializer extends AbstractListDeserializer {
    private Class _type;

    public CollectionDeserializer(Class type) {
        _type = type;
    }

    public Class getType() {
        return _type;
    }

    public Object readList(AbstractHessianInput in, int length)
            throws IOException {
        return readList(in, length, null);
    }

    @Override
    public Object readList(AbstractHessianInput in, int length, Class<?> expectType) throws IOException {
        Collection list = createList();

        in.addRef(list);

        Deserializer deserializer = null;

        SerializerFactory factory = findSerializerFactory(in);
        if(expectType != null){
            deserializer = factory.getDeserializer(expectType.getName());
        }

        while (!in.isEnd())
            list.add(deserializer != null ? deserializer.readObject(in) : in.readObject());

        in.readEnd();

        return list;
    }

    public Object readLengthList(AbstractHessianInput in, int length)
            throws IOException {
        return readList(in, length, null);
    }

    @Override
    public Object readLengthList(AbstractHessianInput in, int length, Class<?> expectType) throws IOException {
        Collection list = createList();

        in.addRef(list);

        Deserializer deserializer = null;

        SerializerFactory factory = findSerializerFactory(in);
        if(expectType != null){
            deserializer = factory.getDeserializer(expectType.getName());
        }

        for (; length > 0; length--)
            list.add(deserializer != null ? deserializer.readObject(in) : in.readObject());

        return list;
    }

    private Collection createList()
            throws IOException {
        Collection list = null;

        if (_type == null)
            list = new ArrayList();
        else if (!_type.isInterface()) {
            try {
                list = (Collection) _type.newInstance();
            } catch (Exception e) {
            }
        }

        if (list != null) {
        } else if (SortedSet.class.isAssignableFrom(_type))
            list = new TreeSet();
        else if (Set.class.isAssignableFrom(_type))
            list = new HashSet();
        else if (List.class.isAssignableFrom(_type))
            list = new ArrayList();
        else if (Collection.class.isAssignableFrom(_type))
            list = new ArrayList();
        else {
            try {
                list = (Collection) _type.newInstance();
            } catch (Exception e) {
                throw new IOExceptionWrapper(e);
            }
        }

        return list;
    }
}


