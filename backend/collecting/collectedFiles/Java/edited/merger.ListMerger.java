

package com.alibaba.dubbo.rpc.cluster.merger;

import com.alibaba.dubbo.rpc.cluster.Merger;

import java.util.ArrayList;
import java.util.List;

public class ListMerger implements Merger<List<?>> {

    public List<Object> merge(List<?>... items) {
        List<Object> result = new ArrayList<Object>();
        for (List<?> item : items) {
            if (item != null) {
                result.addAll(item);
            }
        }
        return result;
    }

}
