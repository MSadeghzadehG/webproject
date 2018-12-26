
package com.alibaba.dubbo.common.extensionloader.ext6_inject.impl;

import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.common.extensionloader.ext6_inject.Ext6;

import java.util.List;

public class Ext6Impl2 implements Ext6 {
    List<String> list;

    public List<String> getList() {
        return list;
    }

    public void setList(List<String> list) {
        this.list = list;
    }

    public String echo(URL url, String s) {
        throw new UnsupportedOperationException();
    }

}