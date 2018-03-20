

package org.elasticsearch.analysis.common;

import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.plugins.Plugin;
import org.elasticsearch.test.ESSingleNodeTestCase;

import java.util.Collection;
import java.util.Collections;

public class MassiveWordListTests extends ESSingleNodeTestCase {

    @Override
    protected Collection<Class<? extends Plugin>> getPlugins() {
        return Collections.singleton(CommonAnalysisPlugin.class);
    }

    public void testCreateIndexWithMassiveWordList() {
        String[] wordList = new String[100000];
        for (int i = 0; i < wordList.length; i++) {
            wordList[i] = "hello world";
        }
        client().admin().indices().prepareCreate("test").setSettings(Settings.builder()
            .put("index.number_of_shards", 1)
            .put("analysis.analyzer.test_analyzer.type", "custom")
            .put("analysis.analyzer.test_analyzer.tokenizer", "standard")
            .putList("analysis.analyzer.test_analyzer.filter", "dictionary_decompounder", "lowercase")
            .put("analysis.filter.dictionary_decompounder.type", "dictionary_decompounder")
            .putList("analysis.filter.dictionary_decompounder.word_list", wordList)
        ).get();
    }
}
