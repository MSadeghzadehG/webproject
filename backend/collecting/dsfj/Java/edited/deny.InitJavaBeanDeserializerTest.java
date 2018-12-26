package com.alibaba.json.bvt.parser.deser.deny;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONException;
import com.alibaba.fastjson.parser.ParserConfig;
import junit.framework.TestCase;

import java.util.Properties;


public class InitJavaBeanDeserializerTest extends TestCase {
    ParserConfig config = new ParserConfig();

    protected void setUp() throws Exception {
        assertFalse(config.isAutoTypeSupport());

        Properties properties = new Properties();
        properties.put(ParserConfig.AUTOTYPE_SUPPORT_PROPERTY, "false");
                        config.configFromPropety(properties);

        assertFalse(config.isAutoTypeSupport());
    }

    public void test_desktop() throws Exception {
        DenyTest11.Model model = new DenyTest11.Model();
        model.a = new DenyTest11.B();
        String text = "{\"@type\":\"com.alibaba.json.bvt.parser.deser.deny.InitJavaBeanDeserializerTest$Model\"}";

        Exception error = null;
        try {
            Object obj = JSON.parseObject(text, Object.class, config);
            System.out.println(obj.getClass());
        } catch (JSONException ex) {
            error = ex;
        }
        assertNotNull(error);

        config.initJavaBeanDeserializers(Model.class);

        Object obj = JSON.parseObject(text, Object.class, config);
        assertEquals(Model.class, obj.getClass());
    }

    public static class Model {

    }
}
