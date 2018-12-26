

package com.alibaba.com.caucho.hessian.io;

import java.io.IOException;


public class AbstractListDeserializer extends AbstractDeserializer {
    public Object readObject(AbstractHessianInput in)
            throws IOException {
        Object obj = in.readObject();

        if (obj != null)
            throw error("expected list at " + obj.getClass().getName() + " (" + obj + ")");
        else
            throw error("expected list at null");
    }
}
