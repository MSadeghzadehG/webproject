

package org.elasticsearch.analysis.common;

import org.apache.lucene.analysis.TokenStream;
import org.apache.lucene.analysis.cjk.CJKBigramFilter;
import org.apache.lucene.analysis.miscellaneous.DisableGraphAttribute;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.env.Environment;
import org.elasticsearch.index.IndexSettings;
import org.elasticsearch.index.analysis.AbstractTokenFilterFactory;

import java.util.Arrays;
import java.util.HashSet;
import java.util.List;
import java.util.Set;


public final class CJKBigramFilterFactory extends AbstractTokenFilterFactory {

    private final int flags;
    private final boolean outputUnigrams;

    CJKBigramFilterFactory(IndexSettings indexSettings, Environment environment, String name, Settings settings) {
        super(indexSettings, name, settings);
        outputUnigrams = settings.getAsBoolean("output_unigrams", false);
        final List<String> asArray = settings.getAsList("ignored_scripts");
        Set<String> scripts = new HashSet<>(Arrays.asList("han", "hiragana", "katakana", "hangul"));
        if (asArray != null) {
            scripts.removeAll(asArray);
        }
        int flags = 0;
        for (String script : scripts) {
            if ("han".equals(script)) {
                flags |= CJKBigramFilter.HAN;
            } else if ("hiragana".equals(script)) {
                flags |= CJKBigramFilter.HIRAGANA;
            } else if ("katakana".equals(script)) {
                flags |= CJKBigramFilter.KATAKANA;
            } else if ("hangul".equals(script)) {
                flags |= CJKBigramFilter.HANGUL;
            }
        }
        this.flags = flags;
    }

    @Override
    public TokenStream create(TokenStream tokenStream) {
        CJKBigramFilter filter = new CJKBigramFilter(tokenStream, flags, outputUnigrams);
        if (outputUnigrams) {
            
            filter.addAttribute(DisableGraphAttribute.class);
        }
        return filter;
    }

}
