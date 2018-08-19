

package org.elasticsearch.action;

import org.elasticsearch.common.unit.TimeValue;

import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;


public interface ActionFuture<T> extends Future<T> {

    
    T actionGet();

    
    T actionGet(String timeout);

    
    T actionGet(long timeoutMillis);

    
    T actionGet(long timeout, TimeUnit unit);

    
    T actionGet(TimeValue timeout);
}
