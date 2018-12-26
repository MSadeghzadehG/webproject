

package org.elasticsearch.cluster;

import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.common.io.stream.Writeable;

import java.io.IOException;


public interface Diffable<T> extends Writeable {

    
    Diff<T> diff(T previousState);

}
