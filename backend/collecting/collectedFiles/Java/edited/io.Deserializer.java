

package com.alibaba.com.caucho.hessian.io;

import java.io.IOException;


public interface Deserializer {
    public Class getType();

    public Object readObject(AbstractHessianInput in)
            throws IOException;

    public Object readList(AbstractHessianInput in, int length)
            throws IOException;

    
    public Object readList(AbstractHessianInput in, int length, Class<?> expectType)
        throws IOException;

    public Object readLengthList(AbstractHessianInput in, int length)
            throws IOException;

    
    public Object readLengthList(AbstractHessianInput in, int length, Class<?> expectType)
        throws IOException;

    public Object readMap(AbstractHessianInput in)
            throws IOException;

    
    public Object readMap(AbstractHessianInput in, Class<?> expectKeyType, Class<?> expectValueType )
        throws IOException;

    public Object readObject(AbstractHessianInput in, String[] fieldNames)
            throws IOException;
}
