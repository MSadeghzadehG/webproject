
package com.alibaba.dubbo.common.utils;

import junit.framework.TestCase;
import org.junit.Test;

import java.util.ArrayList;
import java.util.List;

public class StringUtilsTest extends TestCase {
    public void testJoin() throws Exception {
        String[] s = {"1", "2", "3"};
        assertEquals(StringUtils.join(s), "123");
        assertEquals(StringUtils.join(s, ','), "1,2,3");
    }

    public void testSplit() throws Exception {
        String s = "d,1,2,4";
        assertEquals(StringUtils.split(s, ',').length, 4);
    }

    public void testTranslat() throws Exception {
        String s = "16314";
        assertEquals(StringUtils.translat(s, "123456", "abcdef"), "afcad");
        assertEquals(StringUtils.translat(s, "123456", "abcd"), "acad");
    }

    public void testJoin_Colletion_String() throws Exception {
        List<String> list = new ArrayList<String>();
        assertEquals("", StringUtils.join(list, ","));

        list.add("v1");
        assertEquals("v1", StringUtils.join(list, "-"));

        list.add("v2");
        list.add("v3");
        String out = StringUtils.join(list, ":");
        assertEquals("v1:v2:v3", out);
    }

    @Test
    public void testCamelToSplitName() throws Exception {
        assertEquals("ab-cd-ef", StringUtils.camelToSplitName("abCdEf", "-"));
        assertEquals("ab-cd-ef", StringUtils.camelToSplitName("AbCdEf", "-"));
        assertEquals("ab-cd-ef", StringUtils.camelToSplitName("ab-cd-ef", "-"));
        assertEquals("abcdef", StringUtils.camelToSplitName("abcdef", "-"));
    }
}