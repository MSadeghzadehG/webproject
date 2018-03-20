

package com.alibaba.com.caucho.hessian.io;

import java.io.IOException;
import java.io.InputStream;


public class InputStreamSerializer extends AbstractSerializer {
    public InputStreamSerializer() {
    }

    public void writeObject(Object obj, AbstractHessianOutput out)
            throws IOException {
        InputStream is = (InputStream) obj;

        if (is == null)
            out.writeNull();
        else {
            byte[] buf = new byte[1024];
            int len;

            while ((len = is.read(buf, 0, buf.length)) > 0) {
                out.writeByteBufferPart(buf, 0, len);
            }

            out.writeByteBufferEnd(buf, 0, 0);
        }
    }
}
