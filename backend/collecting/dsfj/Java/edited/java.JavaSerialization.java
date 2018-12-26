
package com.alibaba.dubbo.common.serialize.java;

import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.common.serialize.ObjectInput;
import com.alibaba.dubbo.common.serialize.ObjectOutput;
import com.alibaba.dubbo.common.serialize.Serialization;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class JavaSerialization implements Serialization {

    public byte getContentTypeId() {
        return 3;
    }

    public String getContentType() {
        return "x-application/java";
    }

    public ObjectOutput serialize(URL url, OutputStream out) throws IOException {
        return new JavaObjectOutput(out);
    }

    public ObjectInput deserialize(URL url, InputStream is) throws IOException {
        return new JavaObjectInput(is);
    }

}