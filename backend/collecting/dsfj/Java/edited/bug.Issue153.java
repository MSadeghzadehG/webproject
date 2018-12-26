package com.alibaba.json.bvt.bug;

import java.util.HashMap;

import junit.framework.TestCase;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.TypeReference;


public class Issue153 extends TestCase {
    public void test_for_issue() throws Exception {
        String text = "[{\"url_short\":\"http:        
        JSON.parseObject(text, new TypeReference<HashMap<String, Object>[]>(){});
    }
}
