
package org.elasticsearch.index;

import java.util.Locale;

public enum SlowLogLevel {
    WARN, TRACE, INFO, DEBUG;
    public static SlowLogLevel parse(String level) {
        return valueOf(level.toUpperCase(Locale.ROOT));
    }
}
