
package com.alibaba.remoting.transport.mina;

import java.io.Serializable;


public class World implements Serializable {

    private static final long serialVersionUID = 8563900571013747774L;

    private String name;

    public World() {
    }

    public World(String name) {
        this.name = name;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

}