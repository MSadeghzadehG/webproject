
package com.alibaba.dubbo.monitor.dubbo;

import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.monitor.MonitorService;

import java.util.Arrays;
import java.util.List;


public class MockMonitorService implements MonitorService {

    private URL statistics;

    public void collect(URL statistics) {
        this.statistics = statistics;
    }

    public URL getStatistics() {
        return statistics;
    }

    public List<URL> lookup(URL query) {
        return Arrays.asList(statistics);
    }

}
