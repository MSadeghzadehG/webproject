
package com.alibaba.dubbo.config.api;

import java.util.List;



public interface DemoService {

    String sayName(String name);

    Box getBox();

    void throwDemoException() throws DemoException;

    List<User> getUsers(List<User> users);

    int echo(int i);

}