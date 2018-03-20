

package com.alibaba.com.caucho.hessian.io;

import java.io.IOException;


abstract public class AbstractDeserializer implements Deserializer {
    public Class getType() {
        return Object.class;
    }

    public Object readObject(AbstractHessianInput in)
            throws IOException {
        Object obj = in.readObject();

        String className = getClass().getName();

        if (obj != null)
            throw error(className + ": unexpected object " + obj.getClass().getName() + " (" + obj + ")");
        else
            throw error(className + ": unexpected null value");
    }

    public Object readList(AbstractHessianInput in, int length)
            throws IOException {
        throw new UnsupportedOperationException(String.valueOf(this));
    }

    @Override
    public Object readList(AbstractHessianInput in, int length, Class<?> expectType) throws IOException {
        if(expectType == null) {
            return readList(in, length);
        }
        throw new UnsupportedOperationException(String.valueOf(this));
    }

    public Object readLengthList(AbstractHessianInput in, int length)
            throws IOException {
        throw new UnsupportedOperationException(String.valueOf(this));
    }

    @Override
    public Object readLengthList(AbstractHessianInput in, int length, Class<?> expectType) throws IOException {
        if(expectType == null){
            return readLengthList(in , length);
        }
        throw new UnsupportedOperationException(String.valueOf(this));
    }

    public Object readMap(AbstractHessianInput in)
            throws IOException {
        Object obj = in.readObject();

        String className = getClass().getName();

        if (obj != null)
            throw error(className + ": unexpected object " + obj.getClass().getName() + " (" + obj + ")");
        else
            throw error(className + ": unexpected null value");
    }

    @Override
    public Object readMap(AbstractHessianInput in, Class<?> expectKeyType, Class<?> expectValueType) throws IOException {
        if(expectKeyType  == null && expectValueType == null){
            return readMap(in);
        }
        throw new UnsupportedOperationException(String.valueOf(this));
    }

    public Object readObject(AbstractHessianInput in, String[] fieldNames)
            throws IOException {
        throw new UnsupportedOperationException(String.valueOf(this));
    }

    protected HessianProtocolException error(String msg) {
        return new HessianProtocolException(msg);
    }

    protected String codeName(int ch) {
        if (ch < 0)
            return "end of file";
        else
            return "0x" + Integer.toHexString(ch & 0xff);
    }

    protected SerializerFactory findSerializerFactory(AbstractHessianInput in) {
        SerializerFactory serializerFactory = null;
        if(in instanceof Hessian2Input) {
            serializerFactory = ((Hessian2Input) in).findSerializerFactory();
        }
        else if(in instanceof HessianInput) {
            serializerFactory = ((HessianInput) in).getSerializerFactory();
        }
        return serializerFactory == null? new SerializerFactory(): serializerFactory;
    }
}
