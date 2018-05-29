
package org.elasticsearch.xpack.core.watcher.condition;

import org.elasticsearch.common.xcontent.XContentParser;

import java.io.IOException;
import java.time.Clock;


public interface ConditionFactory {

    
    ExecutableCondition parse(Clock clock, String watchId, XContentParser parser) throws IOException;

}
