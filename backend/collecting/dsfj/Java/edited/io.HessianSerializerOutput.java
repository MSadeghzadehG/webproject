

package com.alibaba.com.caucho.hessian.io;

import java.io.IOException;
import java.io.OutputStream;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;


public class HessianSerializerOutput extends HessianOutput {
    
    public HessianSerializerOutput(OutputStream os) {
        super(os);
    }

    
    public HessianSerializerOutput() {
    }

    
    public void writeObjectImpl(Object obj)
            throws IOException {
        Class cl = obj.getClass();

        try {
            Method method = cl.getMethod("writeReplace", new Class[0]);
            Object repl = method.invoke(obj, new Object[0]);

            writeObject(repl);
            return;
        } catch (Exception e) {
        }

        try {
            writeMapBegin(cl.getName());
            for (; cl != null; cl = cl.getSuperclass()) {
                Field[] fields = cl.getDeclaredFields();
                for (int i = 0; i < fields.length; i++) {
                    Field field = fields[i];

                    if (Modifier.isTransient(field.getModifiers()) ||
                            Modifier.isStatic(field.getModifiers()))
                        continue;

                                        field.setAccessible(true);

                    writeString(field.getName());
                    writeObject(field.get(obj));
                }
            }
            writeMapEnd();
        } catch (IllegalAccessException e) {
            throw new IOExceptionWrapper(e);
        }
    }
}
