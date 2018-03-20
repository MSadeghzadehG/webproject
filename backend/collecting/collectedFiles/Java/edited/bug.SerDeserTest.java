
package com.alibaba.json.bvt.bug;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

import junit.framework.TestCase;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.TypeReference;
import com.alibaba.fastjson.parser.ParserConfig;
import com.alibaba.fastjson.serializer.SerializerFeature;
import com.alibaba.json.bvtVO.OptionKey;
import com.alibaba.json.bvtVO.OptionValue;
import com.alibaba.json.bvtVO.TempAttachMetaOption;


public class SerDeserTest extends TestCase {
    protected void setUp() throws Exception {
        com.alibaba.fastjson.parser.ParserConfig.global.addAccept("com.alibaba.json.bvtVO.");
    }

    
    private static Map<OptionKey, OptionValue<?>> options;

    static {
        options = new HashMap<OptionKey, OptionValue<?>>();

        TempAttachMetaOption attach = new TempAttachMetaOption();
        attach.setId(1000);
        attach.setName("test_name");
        attach.setPath("http:
        ArrayList<TempAttachMetaOption> attachList = new ArrayList<TempAttachMetaOption>();
        attachList.add(attach);

                OptionValue<ArrayList<TempAttachMetaOption>> optionValue = new OptionValue<ArrayList<TempAttachMetaOption>>();
        optionValue.setValue(attachList);

        options.put(OptionKey.TEMPALTE_ATTACH_META, optionValue);
    }

    public void test_for_yaolei() {
                String jsonString = JSON.toJSONString(options);
        System.out.println(jsonString);
        {
                        HashMap<OptionKey, OptionValue<?>> deserOptions = (HashMap<OptionKey, OptionValue<?>>) JSON.parseObject(jsonString,
                                                                                                                    new TypeReference<HashMap<OptionKey, OptionValue<?>>>() {

                                                                                                                    });
            System.out.println(deserOptions.get(OptionKey.TEMPALTE_ATTACH_META));
        }

                jsonString = JSON.toJSONString(options, SerializerFeature.WriteClassName);
        System.out.println(jsonString);
                HashMap<OptionKey, OptionValue<?>> deserOptions = (HashMap<OptionKey, OptionValue<?>>) JSON.parse(jsonString);
        System.out.println(deserOptions.get(OptionKey.TEMPALTE_ATTACH_META));
    }
}
