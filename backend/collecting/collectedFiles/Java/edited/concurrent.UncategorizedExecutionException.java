

package org.elasticsearch.common.util.concurrent;

import org.elasticsearch.ElasticsearchException;
import org.elasticsearch.common.io.stream.StreamInput;

import java.io.IOException;

public class UncategorizedExecutionException extends ElasticsearchException {

    public UncategorizedExecutionException(String msg, Throwable cause) {
        super(msg, cause);
    }

    public UncategorizedExecutionException(StreamInput in) throws IOException{
        super(in);
    }
}
