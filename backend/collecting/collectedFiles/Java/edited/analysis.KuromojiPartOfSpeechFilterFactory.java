

package org.elasticsearch.index.analysis;

import org.apache.lucene.analysis.TokenStream;
import org.apache.lucene.analysis.ja.JapaneseAnalyzer;
import org.apache.lucene.analysis.ja.JapanesePartOfSpeechStopFilter;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.env.Environment;
import org.elasticsearch.index.IndexSettings;

import java.util.HashSet;
import java.util.List;
import java.util.Set;

public class KuromojiPartOfSpeechFilterFactory extends AbstractTokenFilterFactory {

    private final Set<String> stopTags = new HashSet<>();

    public KuromojiPartOfSpeechFilterFactory(IndexSettings indexSettings, Environment env, String name, Settings settings) {
        super(indexSettings, name, settings);
        List<String> wordList = Analysis.getWordList(env, settings, "stoptags");
        if (wordList != null) {
            stopTags.addAll(wordList);
        } else {
            stopTags.addAll(JapaneseAnalyzer.getDefaultStopTags());
        }
    }

    @Override
    public TokenStream create(TokenStream tokenStream) {
        return new JapanesePartOfSpeechStopFilter(tokenStream, stopTags);
    }

}
