
package com.alibaba.dubbo.common.serialize.support;

import java.util.Collection;


public interface SerializationOptimizer {

    Collection<Class> getSerializableClasses();
}
