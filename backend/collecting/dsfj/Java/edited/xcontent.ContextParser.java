

package org.elasticsearch.common.xcontent;

import java.io.IOException;


@FunctionalInterface
public interface ContextParser<Context, T> {
    T parse(XContentParser p, Context c) throws IOException;
}
