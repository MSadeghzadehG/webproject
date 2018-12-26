

package org.elasticsearch.search.sort;

import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.common.io.stream.StreamOutput;
import org.elasticsearch.common.io.stream.Writeable;

import java.io.IOException;
import java.util.Locale;


public enum SortOrder implements Writeable {
    
    ASC {
        @Override
        public String toString() {
            return "asc";
        }
    },
    
    DESC {
        @Override
        public String toString() {
            return "desc";
        }
    };

    public static SortOrder readFromStream(StreamInput in) throws IOException {
        return in.readEnum(SortOrder.class);
    }

    @Override
    public void writeTo(StreamOutput out) throws IOException {
        out.writeEnum(this);
    }

    public static SortOrder fromString(String op) {
        return valueOf(op.toUpperCase(Locale.ROOT));
    }
}
