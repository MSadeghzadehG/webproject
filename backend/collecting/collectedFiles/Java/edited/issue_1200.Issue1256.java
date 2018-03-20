package com.alibaba.json.bvt.issue_1200;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.annotation.JSONField;
import junit.framework.TestCase;

import java.util.HashMap;
import java.util.Map;


public class Issue1256 extends TestCase {
    public void test_for_issue() throws Exception {


        
        A a = new A();
        a.name = "Mike";
        a.age = 39;

        Map<String,Object> map = new HashMap<String,Object>();
        map.put("key_obj",a);
        map.put("key_string","Hello");
        map.put("key_random",-1193959466L);
        map.put("key_int",10000);

        String jsonString = JSON.toJSONString(map);
        assertTrue(jsonString.contains("Mike"));
    }

    public static class A {
        public String name;
        public int age;
    }
}
