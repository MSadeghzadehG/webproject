package com.alibaba.json.bvt.bug;

import com.alibaba.fastjson.JSON;
import junit.framework.TestCase;

import java.util.Date;


public class Issue987 extends TestCase {
    public void test_for_issue() throws Exception {
        String text = "{\"date\":\"2016-11-09T09:57:20.4Z\"}";
        JSON.parseObject(text, Model.class);
    }

    public static class Model {
        public Date date;
    }
}
