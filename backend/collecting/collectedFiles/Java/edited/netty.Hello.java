
package com.alibaba.dubbo.remoting.transport.netty;

import java.io.Serializable;


public class Hello implements Serializable {

    private static final long serialVersionUID = 8563900571013747774L;

    private String name;

    public Hello() {
    }

    public Hello(String name) {
        this.name = name;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

}