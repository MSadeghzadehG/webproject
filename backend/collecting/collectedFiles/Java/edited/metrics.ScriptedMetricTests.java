

package org.elasticsearch.search.aggregations.metrics;

import org.elasticsearch.script.Script;
import org.elasticsearch.script.ScriptType;
import org.elasticsearch.search.aggregations.BaseAggregationTestCase;
import org.elasticsearch.search.aggregations.metrics.scripted.ScriptedMetricAggregationBuilder;

import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

public class ScriptedMetricTests extends BaseAggregationTestCase<ScriptedMetricAggregationBuilder> {

    @Override
    protected ScriptedMetricAggregationBuilder createTestAggregatorBuilder() {
        ScriptedMetricAggregationBuilder factory = new ScriptedMetricAggregationBuilder(randomAlphaOfLengthBetween(1, 20));
        if (randomBoolean()) {
            factory.initScript(randomScript("initScript"));
        }
        factory.mapScript(randomScript("mapScript"));
        if (randomBoolean()) {
            factory.combineScript(randomScript("combineScript"));
        }
        if (randomBoolean()) {
            factory.reduceScript(randomScript("reduceScript"));
        }
        if (randomBoolean()) {
            Map<String, Object> params = new HashMap<>();
            params.put("foo", "bar");
            factory.params(params);
        }
        return factory;
    }

    private Script randomScript(String script) {
        if (randomBoolean()) {
            return mockScript(script);
        } else {
            ScriptType type = randomFrom(ScriptType.values());
            return new Script(
                type, type == ScriptType.STORED ? null : randomFrom("my_lang", Script.DEFAULT_SCRIPT_LANG), script, Collections.emptyMap());
        }
    }

}
