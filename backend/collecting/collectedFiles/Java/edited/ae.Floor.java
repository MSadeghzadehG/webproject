package com.alibaba.json.bvtVO.ae;

import java.util.List;



public class Floor implements Area {
    public List<Area> children;

    public String name;

    public String getName() {
        return name;
    }
}
