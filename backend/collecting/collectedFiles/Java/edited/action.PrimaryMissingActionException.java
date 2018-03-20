

package org.elasticsearch.action;

import org.elasticsearch.ElasticsearchException;
import org.elasticsearch.common.io.stream.StreamInput;

import java.io.IOException;

public class PrimaryMissingActionException extends ElasticsearchException {

    public PrimaryMissingActionException(String message) {
        super(message);
    }

    public PrimaryMissingActionException(StreamInput in) throws IOException {
        super(in);
    }
}
