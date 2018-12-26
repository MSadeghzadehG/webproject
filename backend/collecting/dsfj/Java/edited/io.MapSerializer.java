

package com.alibaba.com.caucho.hessian.io;

import java.io.IOException;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;


public class MapSerializer extends AbstractSerializer {
    private boolean _isSendJavaType = true;

    
    public boolean getSendJavaType() {
        return _isSendJavaType;
    }

    
    public void setSendJavaType(boolean sendJavaType) {
        _isSendJavaType = sendJavaType;
    }

    public void writeObject(Object obj, AbstractHessianOutput out)
            throws IOException {
        if (out.addRef(obj))
            return;

        Map map = (Map) obj;

        Class cl = obj.getClass();

        if (cl.equals(HashMap.class)
                || !_isSendJavaType
                || !(obj instanceof java.io.Serializable))
            out.writeMapBegin(null);
        else
            out.writeMapBegin(obj.getClass().getName());

        Iterator iter = map.entrySet().iterator();
        while (iter.hasNext()) {
            Map.Entry entry = (Map.Entry) iter.next();

            out.writeObject(entry.getKey());
            out.writeObject(entry.getValue());
        }
        out.writeMapEnd();
    }
}
