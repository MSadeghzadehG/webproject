

package org.elasticsearch.search.sort;

import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.common.io.stream.StreamOutput;
import org.elasticsearch.common.io.stream.Writeable;

import java.io.IOException;
import java.util.Locale;
import java.util.Objects;


public enum SortMode implements Writeable {
    
    MIN,
    
    MAX,
    
    SUM,
    
    AVG,
    
    MEDIAN;

    @Override
    public void writeTo(final StreamOutput out) throws IOException {
        out.writeEnum(this);
    }

    public static SortMode readFromStream(StreamInput in) throws IOException {
        return in.readEnum(SortMode.class);
    }

    public static SortMode fromString(final String str) {
        Objects.requireNonNull(str, "input string is null");
        switch (str.toLowerCase(Locale.ROOT)) {
            case ("min"):
                return MIN;
            case ("max"):
                return MAX;
            case ("sum"):
                return SUM;
            case ("avg"):
                return AVG;
            case ("median"):
                return MEDIAN;
            default:
                throw new IllegalArgumentException("Unknown SortMode [" + str + "]");
        }
    }

    @Override
    public String toString() {
        return name().toLowerCase(Locale.ROOT);
    }
}
