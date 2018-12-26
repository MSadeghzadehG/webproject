

package org.elasticsearch.index.analysis;

import java.io.Reader;

public interface CharFilterFactory {

    String name();

    Reader create(Reader reader);
}
