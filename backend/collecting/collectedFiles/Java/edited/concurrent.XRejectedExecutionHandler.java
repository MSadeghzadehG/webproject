

package org.elasticsearch.common.util.concurrent;

import java.util.concurrent.RejectedExecutionHandler;

public interface XRejectedExecutionHandler extends RejectedExecutionHandler {

    
    long rejected();
}
