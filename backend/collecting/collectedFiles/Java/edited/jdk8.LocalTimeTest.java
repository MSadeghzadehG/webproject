package com.alibaba.json.bvt.jdk8;

import java.time.LocalTime;
import java.util.Locale;
import java.util.TimeZone;

import junit.framework.TestCase;

import org.junit.Assert;

import com.alibaba.fastjson.JSON;

public class LocalTimeTest extends TestCase {
    protected void setUp() throws Exception {
        JSON.defaultTimeZone = TimeZone.getTimeZone("Asia/Shanghai");
        JSON.defaultLocale = Locale.CHINA;
    }

    public void test_for_issue() throws Exception {
        VO vo = new VO();
        vo.setDate(LocalTime.now());
        
        String text = JSON.toJSONString(vo);
        System.out.println(text);
        
        VO vo1 = JSON.parseObject(text, VO.class);
        
        Assert.assertEquals(vo.getDate(), vo1.getDate());
    }

    
    public void test_for_long() throws Exception {
        String text= "{\"date\":1511248447740}";
        VO vo =JSON.parseObject(text,VO.class);
        Assert.assertEquals(15, vo.date.getHour());
        Assert.assertEquals(14, vo.date.getMinute());
        Assert.assertEquals(07, vo.date.getSecond());
    }

    public static class VO {

        private LocalTime date;

        public LocalTime getDate() {
            return date;
        }

        public void setDate(LocalTime date) {
            this.date = date;
        }

    }
}