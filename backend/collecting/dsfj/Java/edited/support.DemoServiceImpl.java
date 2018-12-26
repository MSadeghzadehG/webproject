
package com.alibaba.dubbo.rpc.protocol.dubbo.support;

import com.alibaba.dubbo.rpc.RpcContext;

import java.util.Map;
import java.util.Set;



public class DemoServiceImpl implements DemoService {
    public DemoServiceImpl() {
        super();
    }

    public void sayHello(String name) {
        System.out.println("hello " + name);
    }

    public String echo(String text) {
        return text;
    }

    public Map echo(Map map) {
        return map;
    }

    public long timestamp() {
        return System.currentTimeMillis();
    }

    public String getThreadName() {
        return Thread.currentThread().getName();
    }

    public int getSize(String[] strs) {
        if (strs == null)
            return -1;
        return strs.length;
    }

    public int getSize(Object[] os) {
        if (os == null)
            return -1;
        return os.length;
    }

    public Object invoke(String service, String method) throws Exception {
        System.out.println("RpcContext.getContext().getRemoteHost()=" + RpcContext.getContext().getRemoteHost());
        return service + ":" + method;
    }

    public Type enumlength(Type... types) {
        if (types.length == 0)
            return Type.Lower;
        return types[0];
    }

    public Type enumlength(Type type) {
        return type;
    }

    public int stringLength(String str) {
        return str.length();
    }

    public String get(CustomArgument arg1) {
        return arg1.toString();
    }

    public byte getbyte(byte arg) {
        return arg;
    }

    public Person gerPerson(Person person) {
        return person;
    }

    public Set<String> keys(Map<String, String> map) {
        return map == null ? null : map.keySet();
    }

    public void nonSerializedParameter(NonSerialized ns) {
    }

    public NonSerialized returnNonSerialized() {
        return new NonSerialized();
    }
}