
package com.alibaba.dubbo.rpc.protocol.dubbo.support;

import java.util.Map;
import java.util.Set;




public interface DemoService {
    void sayHello(String name);

    Set<String> keys(Map<String, String> map);

    String echo(String text);

    Map echo(Map map);

    long timestamp();

    String getThreadName();

    int getSize(String[] strs);

    int getSize(Object[] os);

    Object invoke(String service, String method) throws Exception;

    int stringLength(String str);

    Type enumlength(Type... types);


    String get(CustomArgument arg1);

    byte getbyte(byte arg);

    void nonSerializedParameter(NonSerialized ns);

    NonSerialized returnNonSerialized();

}