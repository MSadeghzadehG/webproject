

package org.elasticsearch.analysis.common;

import org.apache.lucene.analysis.TokenStream;
import org.apache.lucene.analysis.core.TypeTokenFilter;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.env.Environment;
import org.elasticsearch.index.IndexSettings;
import org.elasticsearch.index.analysis.AbstractTokenFilterFactory;
import org.elasticsearch.index.analysis.TokenFilterFactory;

import java.util.HashSet;
import java.util.List;
import java.util.Set;


public class KeepTypesFilterFactory extends AbstractTokenFilterFactory {
    private final Set<String> keepTypes;
    private static final String KEEP_TYPES_KEY = "types";

    KeepTypesFilterFactory(IndexSettings indexSettings, Environment env, String name, Settings settings) {
        super(indexSettings, name, settings);

        final List<String> arrayKeepTypes = settings.getAsList(KEEP_TYPES_KEY, null);
        if ((arrayKeepTypes == null)) {
            throw new IllegalArgumentException("keep_types requires `" + KEEP_TYPES_KEY + "` to be configured");
        }

        this.keepTypes = new HashSet<>(arrayKeepTypes);
    }

    @Override
    public TokenStream create(TokenStream tokenStream) {
        return new TypeTokenFilter(tokenStream, keepTypes, true);
    }
}
