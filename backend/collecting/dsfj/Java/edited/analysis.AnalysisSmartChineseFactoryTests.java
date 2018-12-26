

package org.elasticsearch.index.analysis;

import org.elasticsearch.indices.analysis.AnalysisFactoryTestCase;
import org.elasticsearch.plugin.analysis.smartcn.AnalysisSmartChinesePlugin;

import java.util.HashMap;
import java.util.Map;

public class AnalysisSmartChineseFactoryTests extends AnalysisFactoryTestCase {
    public AnalysisSmartChineseFactoryTests() {
        super(new AnalysisSmartChinesePlugin());
    }
    @Override
    protected Map<String, Class<?>> getTokenizers() {
        Map<String, Class<?>> tokenizers = new HashMap<>(super.getTokenizers());
        tokenizers.put("hmmchinese", SmartChineseTokenizerTokenizerFactory.class);
        return tokenizers;
    }

}
