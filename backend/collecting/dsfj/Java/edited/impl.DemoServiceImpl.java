
package com.alibaba.dubbo.config.provider.impl;

import com.alibaba.dubbo.config.api.Box;
import com.alibaba.dubbo.config.api.DemoException;
import com.alibaba.dubbo.config.api.DemoService;
import com.alibaba.dubbo.config.api.User;

import java.util.List;


public class DemoServiceImpl implements DemoService {

    public String sayName(String name) {
        return "say:" + name;
    }

    public Box getBox() {
        return null;
    }

    public void throwDemoException() throws DemoException {
        throw new DemoException("DemoServiceImpl");
    }

    public List<User> getUsers(List<User> users) {
        return users;
    }

    public int echo(int i) {
        return i;
    }

}