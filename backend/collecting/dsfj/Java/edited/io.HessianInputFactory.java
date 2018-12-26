

package com.alibaba.com.caucho.hessian.io;

import java.io.IOException;
import java.io.InputStream;
import java.util.logging.Logger;

public class HessianInputFactory {
    public static final Logger log
            = Logger.getLogger(HessianInputFactory.class.getName());

    private SerializerFactory _serializerFactory;

    public SerializerFactory getSerializerFactory() {
        return _serializerFactory;
    }

    public void setSerializerFactory(SerializerFactory factory) {
        _serializerFactory = factory;
    }

    public AbstractHessianInput open(InputStream is)
            throws IOException {
        int code = is.read();

        int major = is.read();
        int minor = is.read();

        switch (code) {
            case 'c':
            case 'C':
            case 'r':
            case 'R':
                if (major >= 2) {
                    AbstractHessianInput in = new Hessian2Input(is);
                    in.setSerializerFactory(_serializerFactory);
                    return in;
                } else {
                    AbstractHessianInput in = new HessianInput(is);
                    in.setSerializerFactory(_serializerFactory);
                    return in;
                }

            default:
                throw new IOException((char) code + " is an unknown Hessian message code.");
        }
    }
}
