

package org.elasticsearch.index.query;

import org.apache.lucene.search.Query;
import org.elasticsearch.index.query.ScriptQueryBuilder.ScriptQuery;
import org.elasticsearch.script.MockScriptEngine;
import org.elasticsearch.script.Script;
import org.elasticsearch.script.ScriptType;
import org.elasticsearch.search.internal.SearchContext;
import org.elasticsearch.test.AbstractQueryTestCase;

import java.io.IOException;
import java.util.Collections;
import java.util.Map;
import java.util.Set;

import static org.hamcrest.Matchers.instanceOf;

public class ScriptQueryBuilderTests extends AbstractQueryTestCase<ScriptQueryBuilder> {
    @Override
    protected ScriptQueryBuilder doCreateTestQueryBuilder() {
        String script = "1";
        Map<String, Object> params = Collections.emptyMap();
        return new ScriptQueryBuilder(new Script(ScriptType.INLINE, MockScriptEngine.NAME, script, params));
    }

    @Override
    protected boolean builderGeneratesCacheableQueries() {
        return false;
    }

    @Override
    protected void doAssertLuceneQuery(ScriptQueryBuilder queryBuilder, Query query, SearchContext context) throws IOException {
        assertThat(query, instanceOf(ScriptQueryBuilder.ScriptQuery.class));
    }

    public void testIllegalConstructorArg() {
        expectThrows(IllegalArgumentException.class, () -> new ScriptQueryBuilder((Script) null));
    }

    public void testFromJsonVerbose() throws IOException {
        String json =
            "{\n" +
                "  \"script\" : {\n" +
                "    \"script\" : {\n" +
                "      \"source\" : \"5\",\n" +
                "      \"lang\" : \"mockscript\"\n" +
                "    },\n" +
                "    \"boost\" : 1.0,\n" +
                "    \"_name\" : \"PcKdEyPOmR\"\n" +
                "  }\n" +
                "}";

        ScriptQueryBuilder parsed = (ScriptQueryBuilder) parseQuery(json);
        checkGeneratedJson(json, parsed);

        assertEquals(json, "mockscript", parsed.script().getLang());
    }

    public void testFromJson() throws IOException {
        String json =
            "{\n" +
                "  \"script\" : {\n" +
                "    \"script\" : \"5\"," +
                "    \"boost\" : 1.0,\n" +
                "    \"_name\" : \"PcKdEyPOmR\"\n" +
                "  }\n" +
                "}";

        ScriptQueryBuilder parsed = (ScriptQueryBuilder) parseQuery(json);
        assertEquals(json, "5", parsed.script().getIdOrCode());
    }

    @Override
    protected Set<String> getObjectsHoldingArbitraryContent() {
                        return Collections.singleton(Script.PARAMS_PARSE_FIELD.getPreferredName());
    }

    @Override
    protected boolean isCachable(ScriptQueryBuilder queryBuilder) {
        return false;
    }
}
