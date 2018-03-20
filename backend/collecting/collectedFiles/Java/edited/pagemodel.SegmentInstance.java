
package com.alibaba.json.test.entity.pagemodel;

import java.io.Serializable;
import java.util.List;


public class SegmentInstance extends ComponentInstance implements Serializable {

    private static final long serialVersionUID = -2307992962779806227L;

    List<LayoutInstance>      layouts;

    public List<LayoutInstance> getLayouts() {
        return layouts;
    }

    public void setLayouts(List<LayoutInstance> layouts) {
        this.layouts = layouts;
    }

}
