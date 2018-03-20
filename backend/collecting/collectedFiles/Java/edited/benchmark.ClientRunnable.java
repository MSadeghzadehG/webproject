
package com.alibaba.dubbo.rpc.benchmark;

import java.util.List;


public interface ClientRunnable extends Runnable {

    public List<long[]> getResults();

}
