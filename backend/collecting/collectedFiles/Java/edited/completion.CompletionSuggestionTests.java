

package org.elasticsearch.search.suggest.completion;

import org.elasticsearch.common.text.Text;
import org.elasticsearch.search.suggest.Suggest;
import org.elasticsearch.test.ESTestCase;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.lessThanOrEqualTo;

public class CompletionSuggestionTests extends ESTestCase {

    public void testToReduce() throws Exception {
        List<Suggest.Suggestion<CompletionSuggestion.Entry>> shardSuggestions = new ArrayList<>();
        int nShards = randomIntBetween(1, 10);
        String name = randomAlphaOfLength(10);
        int size = randomIntBetween(3, 5);
        for (int i = 0; i < nShards; i++) {
            CompletionSuggestion suggestion = new CompletionSuggestion(name, size, false);
            suggestion.addTerm(new CompletionSuggestion.Entry(new Text(""), 0, 0));
            shardSuggestions.add(suggestion);
        }
        int totalResults = randomIntBetween(0, 5) * nShards;
        float maxScore = randomIntBetween(totalResults, totalResults*2);
        for (int i = 0; i < totalResults; i++) {
            Suggest.Suggestion<CompletionSuggestion.Entry> suggestion = randomFrom(shardSuggestions);
            CompletionSuggestion.Entry entry = suggestion.getEntries().get(0);
            if (entry.getOptions().size() < size) {
                entry.addOption(new CompletionSuggestion.Entry.Option(i, new Text(""),
                    maxScore - i, Collections.emptyMap()));
            }
        }
        CompletionSuggestion reducedSuggestion = CompletionSuggestion.reduceTo(shardSuggestions);
        assertNotNull(reducedSuggestion);
        assertThat(reducedSuggestion.getOptions().size(), lessThanOrEqualTo(size));
        int count = 0;
        for (CompletionSuggestion.Entry.Option option : reducedSuggestion.getOptions()) {
            assertThat(option.getDoc().doc, equalTo(count));
            count++;
        }
    }
}
