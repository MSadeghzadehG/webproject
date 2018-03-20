

package org.elasticsearch.index.analysis;

import org.apache.lucene.analysis.Analyzer;
import org.apache.lucene.analysis.BaseTokenStreamTestCase;
import org.apache.lucene.analysis.MockTokenizer;
import org.apache.lucene.analysis.Tokenizer;
import org.elasticsearch.Version;
import org.elasticsearch.cluster.metadata.IndexMetaData;
import org.elasticsearch.common.UUIDs;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.env.Environment;
import org.elasticsearch.env.TestEnvironment;
import org.elasticsearch.index.IndexSettings;
import org.elasticsearch.index.analysis.pl.PolishStemTokenFilterFactory;
import org.elasticsearch.indices.analysis.AnalysisFactoryTestCase;
import org.elasticsearch.plugin.analysis.stempel.AnalysisStempelPlugin;

import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

public class AnalysisPolishFactoryTests extends AnalysisFactoryTestCase {
    public AnalysisPolishFactoryTests() {
        super(new AnalysisStempelPlugin());
    }

    @Override
    protected Map<String, Class<?>> getTokenFilters() {
        Map<String, Class<?>> filters = new HashMap<>(super.getTokenFilters());
        filters.put("stempelpolishstem", PolishStemTokenFilterFactory.class);
        return filters;
    }

    public void testThreadSafety() throws IOException {
                Settings settings = Settings.builder()
                        .put(IndexMetaData.SETTING_VERSION_CREATED, Version.CURRENT)
            .put(IndexMetaData.SETTING_NUMBER_OF_REPLICAS, 0)
            .put(IndexMetaData.SETTING_NUMBER_OF_SHARDS, 1)
            .put(IndexMetaData.SETTING_INDEX_UUID, UUIDs.randomBase64UUID())
            .put(Environment.PATH_HOME_SETTING.getKey(), createTempDir().toString())
            .build();
        Environment environment = TestEnvironment.newEnvironment(settings);
        IndexMetaData metaData = IndexMetaData.builder(IndexMetaData.INDEX_UUID_NA_VALUE).settings(settings).build();
        IndexSettings indexSettings = new IndexSettings(metaData, Settings.EMPTY);
        testThreadSafety(new PolishStemTokenFilterFactory(indexSettings, environment, "stempelpolishstem", settings));
    }

        private void testThreadSafety(TokenFilterFactory factory) throws IOException {
        final Analyzer analyzer = new Analyzer() {
            @Override
            protected TokenStreamComponents createComponents(String fieldName) {
                Tokenizer tokenizer = new MockTokenizer();
                return new TokenStreamComponents(tokenizer, factory.create(tokenizer));
            }
        };
        BaseTokenStreamTestCase.checkRandomData(random(), analyzer, 100);
    }
}
