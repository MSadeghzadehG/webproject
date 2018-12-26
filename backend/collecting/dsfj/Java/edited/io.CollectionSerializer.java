

package com.alibaba.com.caucho.hessian.io;

import java.io.IOException;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;


public class CollectionSerializer extends AbstractSerializer {
    private boolean _sendJavaType = true;

    
    public boolean getSendJavaType() {
        return _sendJavaType;
    }

    
    public void setSendJavaType(boolean sendJavaType) {
        _sendJavaType = sendJavaType;
    }

    public void writeObject(Object obj, AbstractHessianOutput out)
            throws IOException {
        if (out.addRef(obj))
            return;

        Collection list = (Collection) obj;

        Class cl = obj.getClass();
        boolean hasEnd;

        if (cl.equals(ArrayList.class)
                || !_sendJavaType
                || !Serializable.class.isAssignableFrom(cl))
            hasEnd = out.writeListBegin(list.size(), null);
        else
            hasEnd = out.writeListBegin(list.size(), obj.getClass().getName());

        Iterator iter = list.iterator();
        while (iter.hasNext()) {
            Object value = iter.next();

            out.writeObject(value);
        }

        if (hasEnd)
            out.writeListEnd();
    }
}
