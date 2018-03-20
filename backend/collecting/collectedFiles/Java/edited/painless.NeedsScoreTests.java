

package org.elasticsearch.painless;

import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.index.IndexService;
import org.elasticsearch.index.query.QueryShardContext;
import org.elasticsearch.painless.spi.Whitelist;
import org.elasticsearch.script.ExecutableScript;
import org.elasticsearch.script.ScriptContext;
import org.elasticsearch.script.SearchScript;
import org.elasticsearch.search.lookup.SearchLookup;
import org.elasticsearch.test.ESSingleNodeTestCase;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;


public class NeedsScoreTests extends ESSingleNodeTestCase {

    public void testNeedsScores() {
        IndexService index = createIndex("test", Settings.EMPTY, "type", "d", "type=double");

        Map<ScriptContext<?>, List<Whitelist>> contexts = new HashMap<>();
        contexts.put(SearchScript.CONTEXT, Whitelist.BASE_WHITELISTS);
        contexts.put(ExecutableScript.CONTEXT, Whitelist.BASE_WHITELISTS);
        PainlessScriptEngine service = new PainlessScriptEngine(Settings.EMPTY, contexts);

        QueryShardContext shardContext = index.newQueryShardContext(0, null, () -> 0, null);
        SearchLookup lookup = new SearchLookup(index.mapperService(), shardContext::getForField, null);

        SearchScript.Factory factory = service.compile(null, "1.2", SearchScript.CONTEXT, Collections.emptyMap());
        SearchScript.LeafFactory ss = factory.newFactory(Collections.emptyMap(), lookup);
        assertFalse(ss.needs_score());

        factory = service.compile(null, "doc['d'].value", SearchScript.CONTEXT, Collections.emptyMap());
        ss = factory.newFactory(Collections.emptyMap(), lookup);
        assertFalse(ss.needs_score());

        factory = service.compile(null, "1/_score", SearchScript.CONTEXT, Collections.emptyMap());
        ss = factory.newFactory(Collections.emptyMap(), lookup);
        assertTrue(ss.needs_score());

        factory = service.compile(null, "doc['d'].value * _score", SearchScript.CONTEXT, Collections.emptyMap());
        ss = factory.newFactory(Collections.emptyMap(), lookup);
        assertTrue(ss.needs_score());
    }

}
