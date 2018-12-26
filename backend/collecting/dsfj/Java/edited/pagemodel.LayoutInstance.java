package com.alibaba.json.test.entity.pagemodel;

import java.io.Serializable;
import java.util.List;


public class LayoutInstance extends ComponentInstance implements Serializable {

    private static final long    serialVersionUID = -3025232531863199667L;

    private List<RegionInstance> regions;

    public List<RegionInstance> getRegions() {
        return regions;
    }

    public void setRegions(List<RegionInstance> regions) {
        this.regions = regions;
    }

}
