

package org.elasticsearch.transport;

import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;

public interface TransportFuture<V> extends Future<V> {

    
    V txGet();

    
    V txGet(long timeout, TimeUnit unit);
}

