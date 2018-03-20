package com.alibaba.fastjson.deserializer.issue1463.beans;

import java.io.Serializable;


public class Person implements Serializable {

    private static final long serialVersionUID = 248616267815851026L;

    private String name;

    private Integer age;

    public Person(String name, Integer age) {
        this.name = name;
        this.age = age;
    }

    public String getName() {
        return name;
    }

    public Integer getAge() {
        return age;
    }
}
