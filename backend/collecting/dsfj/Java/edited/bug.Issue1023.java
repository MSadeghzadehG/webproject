package com.alibaba.json.bvt.bug;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONArray;
import junit.framework.TestCase;

import javax.xml.datatype.DatatypeFactory;
import javax.xml.datatype.XMLGregorianCalendar;
import java.util.ArrayList;
import java.util.Date;
import java.util.GregorianCalendar;
import java.util.List;


public class Issue1023 extends TestCase {
    public void test_for_issue() throws Exception {
        Date now = new Date();

        GregorianCalendar gregorianCalendar = (GregorianCalendar) GregorianCalendar.getInstance();
        gregorianCalendar.setTime(now);

        XMLGregorianCalendar calendar = DatatypeFactory.newInstance().newXMLGregorianCalendar(gregorianCalendar);

        String jsonString = JSON.toJSONString(calendar);
                calendar = JSON.parseObject(jsonString, XMLGregorianCalendar.class);

        Object toJSON1 = JSON.toJSON(calendar);                                         
        List<XMLGregorianCalendar> calendarList = new ArrayList<XMLGregorianCalendar>();
        calendarList.add(calendar);
        calendarList.add(calendar);
        calendarList.add(calendar);

        Object toJSON2 = JSON.toJSON(calendarList); 
                JSONArray jsonArray = (JSONArray) JSON.toJSON(calendarList);
        jsonString = jsonArray.toJSONString();
        List<XMLGregorianCalendar> calendarList1 = JSONArray.parseArray(jsonString, XMLGregorianCalendar.class);

                        List<XMLGregorianCalendar> calendarList2 = jsonArray.toJavaList(XMLGregorianCalendar.class);
        assertNotNull(calendarList2);
        assertEquals(3, calendarList2.size());
    }
}
