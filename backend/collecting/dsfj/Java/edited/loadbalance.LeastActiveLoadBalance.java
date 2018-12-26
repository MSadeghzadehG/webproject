
package com.alibaba.dubbo.rpc.cluster.loadbalance;

import com.alibaba.dubbo.common.Constants;
import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.rpc.Invocation;
import com.alibaba.dubbo.rpc.Invoker;
import com.alibaba.dubbo.rpc.RpcStatus;

import java.util.List;
import java.util.Random;


public class LeastActiveLoadBalance extends AbstractLoadBalance {

    public static final String NAME = "leastactive";

    private final Random random = new Random();

    protected <T> Invoker<T> doSelect(List<Invoker<T>> invokers, URL url, Invocation invocation) {
        int length = invokers.size();         int leastActive = -1;         int leastCount = 0;         int[] leastIndexs = new int[length];         int totalWeight = 0;         int firstWeight = 0;         boolean sameWeight = true;         for (int i = 0; i < length; i++) {
            Invoker<T> invoker = invokers.get(i);
            int active = RpcStatus.getStatus(invoker.getUrl(), invocation.getMethodName()).getActive();             int weight = invoker.getUrl().getMethodParameter(invocation.getMethodName(), Constants.WEIGHT_KEY, Constants.DEFAULT_WEIGHT);             if (leastActive == -1 || active < leastActive) {                 leastActive = active;                 leastCount = 1;                 leastIndexs[0] = i;                 totalWeight = weight;                 firstWeight = weight;                 sameWeight = true;             } else if (active == leastActive) {                 leastIndexs[leastCount++] = i;                 totalWeight += weight;                                 if (sameWeight && i > 0
                        && weight != firstWeight) {
                    sameWeight = false;
                }
            }
        }
                if (leastCount == 1) {
                        return invokers.get(leastIndexs[0]);
        }
        if (!sameWeight && totalWeight > 0) {
                        int offsetWeight = random.nextInt(totalWeight);
                        for (int i = 0; i < leastCount; i++) {
                int leastIndex = leastIndexs[i];
                offsetWeight -= getWeight(invokers.get(leastIndex), invocation);
                if (offsetWeight <= 0)
                    return invokers.get(leastIndex);
            }
        }
                return invokers.get(leastIndexs[random.nextInt(leastCount)]);
    }
}