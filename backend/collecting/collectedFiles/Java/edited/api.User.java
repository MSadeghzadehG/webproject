
package com.alibaba.dubbo.config.api;

import java.io.Serializable;


public class User implements Serializable {

    private static final long serialVersionUID = 1L;

    private String name;

    public User() {
    }

    public User(String name) {
        this.name = name;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    @Override
    public int hashCode() {
        return name == null ? -1 : name.hashCode();
    }

    @Override
    public boolean equals(Object obj) {
        if (!(obj instanceof User)) {
            return false;
        }
        User other = (User) obj;
        if (this == other) {
            return true;
        }
        if (name != null && other.name != null) {
            return name.equals(other.name);
        }
        return false;
    }

}
