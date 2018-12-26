package com.alibaba.json.bvt.issue_1300;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.parser.Feature;
import com.alibaba.fastjson.serializer.SerializerFeature;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.google.gson.Gson;
import junit.framework.TestCase;
import org.junit.Assert;
import org.junit.Test;

import java.util.Map;
import java.util.TreeMap;


public class Issue1371 extends TestCase {
    private enum Rooms{
        A, B, C, D ,E ;
    }

    public void testFastjsonEnum(){

        Map<Rooms, Rooms> enumMap = new TreeMap<Rooms, Rooms>();

        enumMap.put(Rooms.C, Rooms.D);
        enumMap.put(Rooms.E, Rooms.A);

        Assert.assertEquals(JSON.toJSONString(enumMap, SerializerFeature.WriteNonStringKeyAsString),
                "{\"C\":\"D\",\"E\":\"A\"}");

    }




}
