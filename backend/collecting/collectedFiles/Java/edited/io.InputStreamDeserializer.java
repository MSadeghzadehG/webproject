

package com.alibaba.com.caucho.hessian.io;

import java.io.IOException;


public class InputStreamDeserializer extends AbstractDeserializer {
    public InputStreamDeserializer() {
    }

    public Object readObject(AbstractHessianInput in)
            throws IOException {
        return in.readInputStream();
    }
}
