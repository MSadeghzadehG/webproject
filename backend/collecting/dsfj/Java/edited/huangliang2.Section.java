package com.alibaba.json.bvtVO.ae.huangliang2;

import com.alibaba.fastjson.annotation.JSONType;

import java.util.List;


@JSONType(typeName = "section")
public class Section implements Area {
    public List<Area> children;

    public String type;

    public String templateId;

    @Override
    public String getName() {
        return templateId;
    }
}
