

package com.alibaba.dubbo.common.serialize.nativejava;

import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.common.serialize.ObjectInput;
import com.alibaba.dubbo.common.serialize.ObjectOutput;
import com.alibaba.dubbo.common.serialize.Serialization;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class NativeJavaSerialization implements Serialization {

    public static final String NAME = "nativejava";

    public byte getContentTypeId() {
        return 7;
    }

    public String getContentType() {
        return "x-application/nativejava";
    }

    public ObjectOutput serialize(URL url, OutputStream output) throws IOException {
        return new NativeJavaObjectOutput(output);
    }

    public ObjectInput deserialize(URL url, InputStream input) throws IOException {
        return new NativeJavaObjectInput(input);
    }
}
