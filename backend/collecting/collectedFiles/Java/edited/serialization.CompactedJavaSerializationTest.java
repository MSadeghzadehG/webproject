
package com.alibaba.dubbo.common.serialize.serialization;


import com.alibaba.dubbo.common.serialize.java.CompactedJavaSerialization;

public class CompactedJavaSerializationTest extends AbstractSerializationPersionFailTest {
    {
        serialization = new CompactedJavaSerialization();
    }
}