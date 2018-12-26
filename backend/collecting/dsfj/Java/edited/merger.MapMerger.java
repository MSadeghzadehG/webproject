
package com.alibaba.dubbo.rpc.cluster.merger;

import com.alibaba.dubbo.rpc.cluster.Merger;

import java.util.HashMap;
import java.util.Map;

public class MapMerger implements Merger<Map<?, ?>> {

    public Map<?, ?> merge(Map<?, ?>... items) {
        if (items.length == 0) {
            return null;
        }
        Map<Object, Object> result = new HashMap<Object, Object>();
        for (Map<?, ?> item : items) {
            if (item != null) {
                result.putAll(item);
            }
        }
        return result;
    }

}
