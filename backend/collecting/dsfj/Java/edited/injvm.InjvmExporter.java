
package com.alibaba.dubbo.rpc.protocol.injvm;

import com.alibaba.dubbo.rpc.Exporter;
import com.alibaba.dubbo.rpc.Invoker;
import com.alibaba.dubbo.rpc.protocol.AbstractExporter;

import java.util.Map;


class InjvmExporter<T> extends AbstractExporter<T> {

    private final String key;

    private final Map<String, Exporter<?>> exporterMap;

    InjvmExporter(Invoker<T> invoker, String key, Map<String, Exporter<?>> exporterMap) {
        super(invoker);
        this.key = key;
        this.exporterMap = exporterMap;
        exporterMap.put(key, this);
    }

    public void unexport() {
        super.unexport();
        exporterMap.remove(key);
    }

}