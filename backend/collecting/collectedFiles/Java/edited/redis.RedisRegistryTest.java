
package com.alibaba.dubbo.registry.redis;

import com.alibaba.dubbo.common.URL;

import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;


public class RedisRegistryTest {

    String service = "com.alibaba.dubbo.test.injvmServie";
    URL registryUrl = URL.valueOf("redis:    URL registryUrlWithPasswd = URL.valueOf("zookeeper:    URL serviceUrl = URL.valueOf("redis:            + "?notify=false&methods=test1,test2");
    URL consumerUrl = URL.valueOf("redis:        
    
    @BeforeClass
    public static void setUpBeforeClass() throws Exception {
    }

    
    @Before
    public void setUp() throws Exception {
            }

    
    @Test
    public void testRegister() {
        
    }

    
    @Test
    public void testSubscribe() {
        

    }

    @Test
    public void testRedisPasswd() {
            }

}