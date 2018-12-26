

package org.elasticsearch.common.lease;

import org.elasticsearch.ElasticsearchException;

import java.io.Closeable;


public interface Releasable extends Closeable {

    @Override
    void close();

}
