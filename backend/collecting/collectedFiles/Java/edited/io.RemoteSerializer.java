

package com.alibaba.com.caucho.hessian.io;

import java.io.IOException;


public class RemoteSerializer extends AbstractSerializer {
    public void writeObject(Object obj, AbstractHessianOutput out)
            throws IOException {
                throw new UnsupportedOperationException(getClass().getName());
    }
}
