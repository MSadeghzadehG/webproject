
package org.elasticsearch.action.search;

import org.elasticsearch.common.CheckedRunnable;

import java.io.IOException;
import java.util.Objects;


abstract class SearchPhase implements CheckedRunnable<IOException> {
    private final String name;

    protected SearchPhase(String name) {
        this.name = Objects.requireNonNull(name, "name must not be null");
    }

    
    public String getName() {
        return name;
    }
}
