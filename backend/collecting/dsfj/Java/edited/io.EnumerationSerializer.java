

package com.alibaba.com.caucho.hessian.io;

import java.io.IOException;
import java.util.Enumeration;


public class EnumerationSerializer extends AbstractSerializer {
    private static EnumerationSerializer _serializer;

    public static EnumerationSerializer create() {
        if (_serializer == null)
            _serializer = new EnumerationSerializer();

        return _serializer;
    }

    public void writeObject(Object obj, AbstractHessianOutput out)
            throws IOException {
        Enumeration iter = (Enumeration) obj;

        boolean hasEnd = out.writeListBegin(-1, null);

        while (iter.hasMoreElements()) {
            Object value = iter.nextElement();

            out.writeObject(value);
        }

        if (hasEnd)
            out.writeListEnd();
    }
}
