

package org.elasticsearch.search.suggest;

import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.common.io.stream.StreamOutput;
import org.elasticsearch.common.io.stream.Writeable;

import java.io.IOException;
import java.util.Locale;
import java.util.Objects;


public enum SortBy implements Writeable {
    
    SCORE,
    
    FREQUENCY;

    @Override
    public void writeTo(final StreamOutput out) throws IOException {
        out.writeEnum(this);
    }

    public static SortBy readFromStream(final StreamInput in) throws IOException {
        return in.readEnum(SortBy.class);
    }

    public static SortBy resolve(final String str) {
        Objects.requireNonNull(str, "Input string is null");
        return valueOf(str.toUpperCase(Locale.ROOT));
    }
}
