package com.alibaba.json.bvt.bug;

import junit.framework.TestCase;
import cn.com.tx.domain.notifyDetail.NotifyDetail;
import cn.com.tx.domain.pagination.Pagination;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.TypeReference;

public class Bug_for_generic_1 extends TestCase {

    public void test() {
        String json2 = "{\"@type\":\"cn.com.tx.domain.pagination.Pagination\",\"fromIndex\":0,\"list\":[{\"@type\":\"cn.com.tx.domain.notifyDetail.NotifyDetail\",\"args\":[\"61354557\",\"依依\",\"六\"],\"destId\":60721687,\"detailId\":3155063,\"display\":false,\"foundTime\":{\"@type\":\"java.sql.Timestamp\",\"val\":1344530416000},\"hotId\":0,\"srcId\":1000,\"templateId\":482},{\"@type\":\"cn.com.tx.domain.notifyDetail.NotifyDetail\",\"args\":[\"14527269\",\"懒洋洋\",\"        cn.com.tx.domain.pagination.Pagination<cn.com.tx.domain.notifyDetail.NotifyDetail> pagination = JSON.parseObject(json2,
                                                                                                                         new TypeReference<Pagination<NotifyDetail>>() {
                                                                                                                         });

    }
}
