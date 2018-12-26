

package com.alibaba.com.caucho.hessian.io;

import java.io.IOException;
import java.io.InputStream;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.util.ArrayList;
import java.util.HashMap;


public class HessianSerializerInput extends HessianInput {
    
    public HessianSerializerInput(InputStream is) {
        super(is);
    }

    
    public HessianSerializerInput() {
    }

    
    protected Object readObjectImpl(Class cl)
            throws IOException {
        try {
            Object obj = cl.newInstance();

            if (_refs == null)
                _refs = new ArrayList();
            _refs.add(obj);

            HashMap fieldMap = getFieldMap(cl);

            int code = read();
            for (; code >= 0 && code != 'z'; code = read()) {
                _peek = code;

                Object key = readObject();

                Field field = (Field) fieldMap.get(key);

                if (field != null) {
                    Object value = readObject(field.getType());
                    field.set(obj, value);
                } else {
                    Object value = readObject();
                }
            }

            if (code != 'z')
                throw expect("map", code);

                        try {
                Method method = cl.getMethod("readResolve", new Class[0]);
                return method.invoke(obj, new Object[0]);
            } catch (Exception e) {
            }

            return obj;
        } catch (IOException e) {
            throw e;
        } catch (Exception e) {
            throw new IOExceptionWrapper(e);
        }
    }

    
    protected HashMap getFieldMap(Class cl) {
        HashMap fieldMap = new HashMap();

        for (; cl != null; cl = cl.getSuperclass()) {
            Field[] fields = cl.getDeclaredFields();
            for (int i = 0; i < fields.length; i++) {
                Field field = fields[i];

                if (Modifier.isTransient(field.getModifiers()) ||
                        Modifier.isStatic(field.getModifiers()))
                    continue;

                                field.setAccessible(true);

                fieldMap.put(field.getName(), field);
            }
        }

        return fieldMap;
    }
}
