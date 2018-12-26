
package com.alibaba.dubbo.common.model;

import java.io.Serializable;
import java.util.Arrays;

public class SerializablePerson implements Serializable {
    private static final long serialVersionUID = 1L;
    byte oneByte = 123;
    private String name = "name1";
    private int age = 11;

    private String[] value = {"value1", "value2"};

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public byte getOneByte() {
        return oneByte;
    }

    public void setOneByte(byte b) {
        this.oneByte = b;
    }

    public int getAge() {
        return age;
    }

    public void setAge(int age) {
        this.age = age;
    }

    public String[] getValue() {
        return value;
    }

    public void setValue(String[] value) {
        this.value = value;
    }

    @Override
    public String toString() {
        return String.format("Person name(%s) age(%d) byte(%s) [value=%s]", name, age, oneByte, Arrays.toString(value));
    }

    @Override
    public int hashCode() {
        final int prime = 31;
        int result = 1;
        result = prime * result + age;
        result = prime * result + ((name == null) ? 0 : name.hashCode());
        result = prime * result + Arrays.hashCode(value);
        return result;
    }

    @Override
    public boolean equals(Object obj) {
        if (this == obj)
            return true;
        if (obj == null)
            return false;
        if (getClass() != obj.getClass())
            return false;
        SerializablePerson other = (SerializablePerson) obj;
        if (age != other.age)
            return false;
        if (name == null) {
            if (other.name != null)
                return false;
        } else if (!name.equals(other.name))
            return false;
        if (!Arrays.equals(value, other.value))
            return false;
        return true;
    }
}