
package com.alibaba.dubbo.common.bytecode;

import junit.framework.TestCase;
import net.sf.cglib.proxy.Enhancer;
import net.sf.cglib.proxy.MethodInterceptor;
import net.sf.cglib.proxy.MethodProxy;
import org.junit.Assert;
import org.junit.Test;

import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Method;

public class ProxyTest extends TestCase {
    public void testMain() throws Exception {
        Proxy proxy = Proxy.getProxy(ITest.class, ITest.class);
        ITest instance = (ITest) proxy.newInstance(new InvocationHandler() {
            public Object invoke(Object proxy, Method method, Object[] args) throws Throwable {
                if ("getName".equals(method.getName())) {
                    assertEquals(args.length, 0);
                } else if ("setName".equals(method.getName())) {
                    assertEquals(args.length, 2);
                    assertEquals(args[0], "qianlei");
                    assertEquals(args[1], "hello");
                }
                return null;
            }
        });

        assertNull(instance.getName());
        instance.setName("qianlei", "hello");
    }

    @Test
    public void testCglibProxy() throws Exception {
        ITest test = (ITest) Proxy.getProxy(ITest.class).newInstance(new InvocationHandler() {

            public Object invoke(Object proxy, Method method, Object[] args) throws Throwable {
                System.out.println(method.getName());
                return null;
            }
        });

        Enhancer enhancer = new Enhancer();
        enhancer.setSuperclass(test.getClass());
        enhancer.setCallback(new MethodInterceptor() {

            public Object intercept(Object obj, Method method, Object[] args, MethodProxy proxy) throws Throwable {
                return null;
            }
        });
        try {
            enhancer.create();
        } catch (IllegalArgumentException e) {
            e.printStackTrace();
            Assert.fail();
        }
    }

    public static interface ITest {
        String getName();

        void setName(String name, String name2);
    }
}