

package com.alibaba.com.caucho.hessian.io;

import java.io.IOException;
import java.io.OutputStream;


abstract public class AbstractHessianOutput {
        protected SerializerFactory _serializerFactory;

    
    public SerializerFactory getSerializerFactory() {
        return _serializerFactory;
    }

    
    public void setSerializerFactory(SerializerFactory factory) {
        _serializerFactory = factory;
    }

    
    public final SerializerFactory findSerializerFactory() {
        SerializerFactory factory = _serializerFactory;

        if (factory == null)
            _serializerFactory = factory = new SerializerFactory();

        return factory;
    }

    
    public void init(OutputStream os) {
    }

    
    public void call(String method, Object[] args)
            throws IOException {
        int length = args != null ? args.length : 0;

        startCall(method, length);

        for (int i = 0; i < length; i++)
            writeObject(args[i]);

        completeCall();
    }

    
    abstract public void startCall()
            throws IOException;

    
    abstract public void startCall(String method, int length)
            throws IOException;

    
    public void writeHeader(String name)
            throws IOException {
        throw new UnsupportedOperationException(getClass().getSimpleName());
    }

    
    abstract public void writeMethod(String method)
            throws IOException;

    
    abstract public void completeCall()
            throws IOException;

    
    abstract public void writeBoolean(boolean value)
            throws IOException;

    
    abstract public void writeInt(int value)
            throws IOException;

    
    abstract public void writeLong(long value)
            throws IOException;

    
    abstract public void writeDouble(double value)
            throws IOException;

    
    abstract public void writeUTCDate(long time)
            throws IOException;

    
    abstract public void writeNull()
            throws IOException;

    
    abstract public void writeString(String value)
            throws IOException;

    
    abstract public void writeString(char[] buffer, int offset, int length)
            throws IOException;

    
    abstract public void writeBytes(byte[] buffer)
            throws IOException;

    
    abstract public void writeBytes(byte[] buffer, int offset, int length)
            throws IOException;

    
    abstract public void writeByteBufferStart()
            throws IOException;

    
    abstract public void writeByteBufferPart(byte[] buffer,
                                             int offset,
                                             int length)
            throws IOException;

    
    abstract public void writeByteBufferEnd(byte[] buffer,
                                            int offset,
                                            int length)
            throws IOException;

    
    abstract protected void writeRef(int value)
            throws IOException;

    
    abstract public boolean removeRef(Object obj)
            throws IOException;

    
    abstract public boolean replaceRef(Object oldRef, Object newRef)
            throws IOException;

    
    abstract public boolean addRef(Object object)
            throws IOException;

    
    public void resetReferences() {
    }

    
    abstract public void writeObject(Object object)
            throws IOException;

    
    abstract public boolean writeListBegin(int length, String type)
            throws IOException;

    
    abstract public void writeListEnd()
            throws IOException;

    
    abstract public void writeMapBegin(String type)
            throws IOException;

    
    abstract public void writeMapEnd()
            throws IOException;

    
    public int writeObjectBegin(String type)
            throws IOException {
        writeMapBegin(type);

        return -2;
    }

    
    public void writeClassFieldLength(int len)
            throws IOException {
    }

    
    public void writeObjectEnd()
            throws IOException {
    }

    public void writeReply(Object o)
            throws IOException {
        startReply();
        writeObject(o);
        completeReply();
    }


    public void startReply()
            throws IOException {
    }

    public void completeReply()
            throws IOException {
    }

    public void writeFault(String code, String message, Object detail)
            throws IOException {
    }

    public void flush()
            throws IOException {
    }

    public void close()
            throws IOException {
    }
}
