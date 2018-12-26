
package com.alibaba.dubbo.common.serialize.fst;

import com.alibaba.dubbo.common.serialize.support.SerializableClassRegistry;

import org.nustaq.serialization.FSTConfiguration;
import org.nustaq.serialization.FSTObjectInput;
import org.nustaq.serialization.FSTObjectOutput;

import java.io.InputStream;
import java.io.OutputStream;

public class FstFactory {

    private static final FstFactory factory = new FstFactory();

    private final FSTConfiguration conf = FSTConfiguration.createDefaultConfiguration();


    public static FstFactory getDefaultFactory() {
        return factory;
    }

    public FstFactory() {
        for (Class clazz : SerializableClassRegistry.getRegisteredClasses()) {
            conf.registerClass(clazz);
        }
    }

    public FSTObjectOutput getObjectOutput(OutputStream outputStream) {
        return conf.getObjectOutput(outputStream);
    }

    public FSTObjectInput getObjectInput(InputStream inputStream) {
        return conf.getObjectInput(inputStream);
    }
}
