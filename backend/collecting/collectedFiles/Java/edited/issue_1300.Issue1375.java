package com.alibaba.json.bvt.issue_1300;

import com.alibaba.fastjson.serializer.CollectionCodec;
import com.alibaba.fastjson.serializer.SerializeConfig;
import junit.framework.TestCase;

import java.util.LinkedList;


public class Issue1375 extends TestCase {
    public void test_issue() throws Exception {
        assertSame(CollectionCodec.instance
                , SerializeConfig.getGlobalInstance()
                        .getObjectWriter(LinkedList.class));
    }
}
