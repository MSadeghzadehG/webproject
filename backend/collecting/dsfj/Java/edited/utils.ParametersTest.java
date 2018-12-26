
package com.alibaba.dubbo.common.utils;

import junit.framework.TestCase;

import java.util.HashMap;
import java.util.Map;

public class ParametersTest extends TestCase {
    final String ServiceName = "com.alibaba.dubbo.rpc.service.GenericService";
    final String ServiceVersion = "1.0.15";
    final String LoadBalance = "lcr";

    public void testMap2Parameters() throws Exception {
        Map<String, String> map = new HashMap<String, String>();
        map.put("name", "com.alibaba.dubbo.rpc.service.GenericService");
        map.put("version", "1.0.15");
        map.put("lb", "lcr");
        map.put("max.active", "500");
        assertEquals(map.get("name"), ServiceName);
        assertEquals(map.get("version"), ServiceVersion);
        assertEquals(map.get("lb"), LoadBalance);
    }

    public void testString2Parameters() throws Exception {
        String qs = "name=com.alibaba.dubbo.rpc.service.GenericService&version=1.0.15&lb=lcr";
        Map<String, String> map = StringUtils.parseQueryString(qs);
        assertEquals(map.get("name"), ServiceName);
        assertEquals(map.get("version"), ServiceVersion);
        assertEquals(map.get("lb"), LoadBalance);
    }
}