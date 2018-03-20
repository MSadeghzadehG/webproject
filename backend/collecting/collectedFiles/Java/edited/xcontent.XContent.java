

package org.elasticsearch.common.xcontent;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.Reader;
import java.util.Collections;
import java.util.Set;


public interface XContent {

    
    static boolean isStrictDuplicateDetectionEnabled() {
                return Booleans.parseBoolean(System.getProperty("es.xcontent.strict_duplicate_detection", "true"), true);
    }

    
    XContentType type();

    byte streamSeparator();

    
    default XContentGenerator createGenerator(OutputStream os) throws IOException {
        return createGenerator(os, Collections.emptySet(), Collections.emptySet());
    }

    
    XContentGenerator createGenerator(OutputStream os, Set<String> includes, Set<String> excludes) throws IOException;

    
    XContentParser createParser(NamedXContentRegistry xContentRegistry,
            DeprecationHandler deprecationHandler, String content) throws IOException;

    
    XContentParser createParser(NamedXContentRegistry xContentRegistry,
            DeprecationHandler deprecationHandler, InputStream is) throws IOException;

    
    XContentParser createParser(NamedXContentRegistry xContentRegistry,
            DeprecationHandler deprecationHandler, byte[] data) throws IOException;

    
    XContentParser createParser(NamedXContentRegistry xContentRegistry,
            DeprecationHandler deprecationHandler, byte[] data, int offset, int length) throws IOException;

    
    XContentParser createParser(NamedXContentRegistry xContentRegistry,
            DeprecationHandler deprecationHandler, Reader reader) throws IOException;
}
