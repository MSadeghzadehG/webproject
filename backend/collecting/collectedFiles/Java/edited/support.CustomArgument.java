
package com.alibaba.dubbo.rpc.protocol.dubbo.support;

import java.io.Serializable;


@SuppressWarnings("serial")
public class CustomArgument implements Serializable {

    Type type;
    String name;

    public CustomArgument() {
    }
    public CustomArgument(Type type, String name) {
        super();
        this.type = type;
        this.name = name;
    }

    public Type getType() {
        return type;
    }

    public void setType(Type type) {
        this.type = type;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }
}