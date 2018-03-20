

package org.elasticsearch.plugin.analysis.smartcn;

import org.apache.lucene.analysis.Analyzer;
import org.elasticsearch.index.analysis.AnalyzerProvider;
import org.elasticsearch.index.analysis.SmartChineseAnalyzerProvider;
import org.elasticsearch.index.analysis.SmartChineseNoOpTokenFilterFactory;
import org.elasticsearch.index.analysis.SmartChineseTokenizerTokenizerFactory;
import org.elasticsearch.index.analysis.TokenFilterFactory;
import org.elasticsearch.index.analysis.TokenizerFactory;
import org.elasticsearch.indices.analysis.AnalysisModule.AnalysisProvider;
import org.elasticsearch.plugins.AnalysisPlugin;
import org.elasticsearch.plugins.Plugin;

import java.util.HashMap;
import java.util.Map;

import static java.util.Collections.singletonMap;

public class AnalysisSmartChinesePlugin extends Plugin implements AnalysisPlugin {
    @Override
    public Map<String, AnalysisProvider<TokenFilterFactory>> getTokenFilters() {
                return singletonMap("smartcn_word", SmartChineseNoOpTokenFilterFactory::new);
    }

    @Override
    public Map<String, AnalysisProvider<TokenizerFactory>> getTokenizers() {
        Map<String, AnalysisProvider<TokenizerFactory>> extra = new HashMap<>();
        extra.put("smartcn_tokenizer", SmartChineseTokenizerTokenizerFactory::new);
                extra.put("smartcn_sentence", SmartChineseTokenizerTokenizerFactory::new);
        return extra;
    }

    @Override
    public Map<String, AnalysisProvider<AnalyzerProvider<? extends Analyzer>>> getAnalyzers() {
        return singletonMap("smartcn", SmartChineseAnalyzerProvider::new);
    }
}
