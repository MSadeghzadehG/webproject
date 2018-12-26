

package org.elasticsearch.plugin.analysis.stempel;

import org.apache.lucene.analysis.Analyzer;
import org.elasticsearch.index.analysis.AnalyzerProvider;
import org.elasticsearch.index.analysis.TokenFilterFactory;
import org.elasticsearch.index.analysis.pl.PolishAnalyzerProvider;
import org.elasticsearch.index.analysis.pl.PolishStemTokenFilterFactory;
import org.elasticsearch.indices.analysis.AnalysisModule.AnalysisProvider;
import org.elasticsearch.plugins.AnalysisPlugin;
import org.elasticsearch.plugins.Plugin;

import java.util.Map;

import static java.util.Collections.singletonMap;

public class AnalysisStempelPlugin extends Plugin implements AnalysisPlugin {
    @Override
    public Map<String, AnalysisProvider<TokenFilterFactory>> getTokenFilters() {
        return singletonMap("polish_stem", PolishStemTokenFilterFactory::new);
    }

    @Override
    public Map<String, AnalysisProvider<AnalyzerProvider<? extends Analyzer>>> getAnalyzers() {
        return singletonMap("polish", PolishAnalyzerProvider::new);
    }
}
