

package com.alibaba.com.caucho.hessian.io;

import java.io.IOException;


public class ClassSerializer extends AbstractSerializer {
    public void writeObject(Object obj, AbstractHessianOutput out)
            throws IOException {
        Class cl = (Class) obj;

        if (cl == null) {
            out.writeNull();
        } else if (out.addRef(obj)) {
            return;
        } else {
            int ref = out.writeObjectBegin("java.lang.Class");

            if (ref < -1) {
                out.writeString("name");
                out.writeString(cl.getName());
                out.writeMapEnd();
            } else {
                if (ref == -1) {
                    out.writeInt(1);
                    out.writeString("name");
                    out.writeObjectBegin("java.lang.Class");
                }

                out.writeString(cl.getName());
            }
        }
    }
}
