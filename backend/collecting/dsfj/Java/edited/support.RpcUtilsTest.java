
package com.alibaba.dubbo.rpc.support;

import com.alibaba.dubbo.common.Constants;
import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.rpc.Invocation;
import com.alibaba.dubbo.rpc.RpcInvocation;

import junit.framework.Assert;
import org.junit.Test;

import java.util.HashMap;
import java.util.Map;

public class RpcUtilsTest {

    
    @Test
    public void testAttachInvocationIdIfAsync_normal() {
        URL url = URL.valueOf("dubbo:        Map<String, String> attachments = new HashMap<String, String>();
        attachments.put("aa", "bb");
        Invocation inv = new RpcInvocation("test", new Class[]{}, new String[]{}, attachments);
        RpcUtils.attachInvocationIdIfAsync(url, inv);
        long id1 = RpcUtils.getInvocationId(inv);
        RpcUtils.attachInvocationIdIfAsync(url, inv);
        long id2 = RpcUtils.getInvocationId(inv);
        Assert.assertTrue(id1 == id2);         Assert.assertTrue(id1 >= 0);
        Assert.assertEquals("bb", attachments.get("aa"));
    }

    
    @Test
    public void testAttachInvocationIdIfAsync_sync() {
        URL url = URL.valueOf("dubbo:        Invocation inv = new RpcInvocation("test", new Class[]{}, new String[]{});
        RpcUtils.attachInvocationIdIfAsync(url, inv);
        Assert.assertNull(RpcUtils.getInvocationId(inv));
    }

    
    @Test
    public void testAttachInvocationIdIfAsync_nullAttachments() {
        URL url = URL.valueOf("dubbo:        Invocation inv = new RpcInvocation("test", new Class[]{}, new String[]{});
        RpcUtils.attachInvocationIdIfAsync(url, inv);
        Assert.assertTrue(RpcUtils.getInvocationId(inv) >= 0l);
    }

    
    @Test
    public void testAttachInvocationIdIfAsync_forceNotAttache() {
        URL url = URL.valueOf("dubbo:        Invocation inv = new RpcInvocation("test", new Class[]{}, new String[]{});
        RpcUtils.attachInvocationIdIfAsync(url, inv);
        Assert.assertNull(RpcUtils.getInvocationId(inv));
    }

    
    @Test
    public void testAttachInvocationIdIfAsync_forceAttache() {
        URL url = URL.valueOf("dubbo:        Invocation inv = new RpcInvocation("test", new Class[]{}, new String[]{});
        RpcUtils.attachInvocationIdIfAsync(url, inv);
        Assert.assertNotNull(RpcUtils.getInvocationId(inv));
    }
}
