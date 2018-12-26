

package org.elasticsearch;

import org.elasticsearch.common.io.stream.StreamInput;

import java.io.IOException;


public class ElasticsearchTimeoutException extends ElasticsearchException {
    public ElasticsearchTimeoutException(StreamInput in) throws IOException {
        super(in);
    }

    public ElasticsearchTimeoutException(Throwable cause) {
        super(cause);
    }

    public ElasticsearchTimeoutException(String message, Object... args) {
        super(message, args);
    }

    public ElasticsearchTimeoutException(String message, Throwable cause, Object... args) {
        super(message, cause, args);
    }
}