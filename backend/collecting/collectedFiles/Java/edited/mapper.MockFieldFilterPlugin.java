

package org.elasticsearch.index.mapper;

import org.elasticsearch.plugins.MapperPlugin;
import org.elasticsearch.plugins.Plugin;

import java.util.function.Function;
import java.util.function.Predicate;

public class MockFieldFilterPlugin extends Plugin implements MapperPlugin {

    @Override
    public Function<String, Predicate<String>> getFieldFilter() {
                return index -> field -> true;
    }
}
