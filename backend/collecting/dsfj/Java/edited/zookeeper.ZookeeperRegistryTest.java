
package com.alibaba.dubbo.registry.zookeeper;

import com.alibaba.dubbo.common.URL;

import junit.framework.Assert;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;


public class ZookeeperRegistryTest {

    String service = "com.alibaba.dubbo.test.injvmServie";
    URL registryUrl = URL.valueOf("zookeeper:    URL serviceUrl = URL.valueOf("zookeeper:            + "?notify=false&methods=test1,test2");
    URL consumerUrl = URL.valueOf("zookeeper:    
    
    @BeforeClass
    public static void setUpBeforeClass() throws Exception {
    }

    
    @Before
    public void setUp() throws Exception {
            }

    

    @Test
    public void testDefaultPort() {
        Assert.assertEquals("10.20.153.10:2181", ZookeeperRegistry.appendDefaultPort("10.20.153.10:0"));
        Assert.assertEquals("10.20.153.10:2181", ZookeeperRegistry.appendDefaultPort("10.20.153.10"));
    }

    
    @Test
    public void testRegister() {
        
    }

    
    @Test
    public void testSubscribe() {
        

    }

}