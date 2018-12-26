package com.alibaba.json.bvt.bug;

import com.alibaba.fastjson.JSON;
import junit.framework.TestCase;
import org.springframework.remoting.support.RemoteInvocation;

import java.util.Date;


public class Issue363 extends TestCase {
    public void test_for_issue() throws Exception {
        RemoteInvocation remoteInvocation = new RemoteInvocation();
        remoteInvocation.setMethodName("test");
        remoteInvocation.setParameterTypes(new Class[] { int.class, Date.class,
                String.class });
        remoteInvocation.setArguments(new Object[] { 1, new Date(),
                "this is a test" });
        String json = JSON.toJSONString(remoteInvocation);
        remoteInvocation = JSON.parseObject(json, RemoteInvocation.class);
        System.out.println(remoteInvocation);
    }
}
