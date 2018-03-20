

package org.elasticsearch.search.aggregations.metrics.scripted;

import org.elasticsearch.common.geo.GeoPoint;
import org.elasticsearch.common.io.stream.Writeable.Reader;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.script.MockScriptEngine;
import org.elasticsearch.script.Script;
import org.elasticsearch.script.ScriptEngine;
import org.elasticsearch.script.ScriptModule;
import org.elasticsearch.script.ScriptService;
import org.elasticsearch.script.ScriptType;
import org.elasticsearch.search.aggregations.Aggregation.CommonFields;
import org.elasticsearch.search.aggregations.ParsedAggregation;
import org.elasticsearch.search.aggregations.pipeline.PipelineAggregator;
import org.elasticsearch.test.InternalAggregationTestCase;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.function.Predicate;
import java.util.function.Supplier;

public class InternalScriptedMetricTests extends InternalAggregationTestCase<InternalScriptedMetric> {

    private static final String REDUCE_SCRIPT_NAME = "reduceScript";
    private boolean hasReduceScript;
    private Supplier<Object>[] valueTypes;
    private final Supplier<Object>[] leafValueSuppliers = new Supplier[] { () -> randomInt(), () -> randomLong(), () -> randomDouble(),
            () -> randomFloat(), () -> randomBoolean(), () -> randomAlphaOfLength(5), () -> new GeoPoint(randomDouble(), randomDouble()),
            () -> null };
    private final Supplier<Object>[] nestedValueSuppliers = new Supplier[] { () -> new HashMap<String, Object>(),
            () -> new ArrayList<>() };

    @Override
    public void setUp() throws Exception {
        super.setUp();
        hasReduceScript = randomBoolean();
                int levels = randomIntBetween(1, 3);
        valueTypes = new Supplier[levels];
        for (int i = 0; i < levels; i++) {
            if (i < levels - 1) {
                valueTypes[i] = randomFrom(nestedValueSuppliers);
            } else {
                                valueTypes[i] = randomFrom(leafValueSuppliers);
            }
        }
    }

    @Override
    protected InternalScriptedMetric createTestInstance(String name, List<PipelineAggregator> pipelineAggregators,
            Map<String, Object> metaData) {
        Map<String, Object> params = new HashMap<>();
        if (randomBoolean()) {
            params.put(randomAlphaOfLength(5), randomAlphaOfLength(5));
        }
        Script reduceScript = null;
        if (hasReduceScript) {
            reduceScript = new Script(ScriptType.INLINE, MockScriptEngine.NAME, REDUCE_SCRIPT_NAME, params);
        }
        Object randomValue = randomValue(valueTypes, 0);
        return new InternalScriptedMetric(name, randomValue, reduceScript, pipelineAggregators, metaData);
    }

    @SuppressWarnings("unchecked")
    private static Object randomValue(Supplier<Object>[] valueTypes, int level) {
        Object value = valueTypes[level].get();
        if (value instanceof Map) {
            int elements = randomIntBetween(1, 5);
            Map<String, Object> map = (Map<String, Object>) value;
            for (int i = 0; i < elements; i++) {
                map.put(randomAlphaOfLength(5), randomValue(valueTypes, level + 1));
            }
        } else if (value instanceof List) {
            int elements = randomIntBetween(1,5);
            List<Object> list = (List<Object>) value;
            for (int i = 0; i < elements; i++) {
                list.add(randomValue(valueTypes, level + 1));
            }
        }
        return value;
    }

    
    @Override
    protected ScriptService mockScriptService() {
                @SuppressWarnings("unchecked")
        MockScriptEngine scriptEngine = new MockScriptEngine(MockScriptEngine.NAME,
                Collections.singletonMap(REDUCE_SCRIPT_NAME, script -> ((List<Object>) script.get("_aggs")).size()));
        Map<String, ScriptEngine> engines = Collections.singletonMap(scriptEngine.getType(), scriptEngine);
        return new ScriptService(Settings.EMPTY, engines, ScriptModule.CORE_CONTEXTS);
    }

    @Override
    protected void assertReduced(InternalScriptedMetric reduced, List<InternalScriptedMetric> inputs) {
        InternalScriptedMetric firstAgg = inputs.get(0);
        assertEquals(firstAgg.getName(), reduced.getName());
        assertEquals(firstAgg.pipelineAggregators(), reduced.pipelineAggregators());
        assertEquals(firstAgg.getMetaData(), reduced.getMetaData());
        if (hasReduceScript) {
            assertEquals(inputs.size(), reduced.aggregation());
        } else {
            assertEquals(inputs.size(), ((List<Object>) reduced.aggregation()).size());
        }
    }

    @Override
    protected Reader<InternalScriptedMetric> instanceReader() {
        return InternalScriptedMetric::new;
    }

    @Override
    protected void assertFromXContent(InternalScriptedMetric aggregation, ParsedAggregation parsedAggregation) {
        assertTrue(parsedAggregation instanceof ParsedScriptedMetric);
        ParsedScriptedMetric parsed = (ParsedScriptedMetric) parsedAggregation;

        assertValues(aggregation.aggregation(), parsed.aggregation());
    }

    private static void assertValues(Object expected, Object actual) {
        if (expected instanceof Long) {
                        if (actual instanceof Integer) {
                assertEquals(((Long) expected).intValue(), actual);
            } else {
                assertEquals(expected, actual);
            }
        } else if (expected instanceof Float) {
                        if (actual instanceof Double) {
                assertEquals(expected, ((Double) actual).floatValue());
            } else {
                assertEquals(expected, actual);
            }
        } else if (expected instanceof GeoPoint) {
            assertTrue(actual instanceof Map);
            GeoPoint point = (GeoPoint) expected;
            Map<String, Object> pointMap = (Map<String, Object>) actual;
            assertEquals(point.getLat(), pointMap.get("lat"));
            assertEquals(point.getLon(), pointMap.get("lon"));
        } else if (expected instanceof Map) {
            Map<String, Object> expectedMap = (Map<String, Object>) expected;
            Map<String, Object> actualMap = (Map<String, Object>) actual;
            assertEquals(expectedMap.size(), actualMap.size());
            for (String key : expectedMap.keySet()) {
                assertValues(expectedMap.get(key), actualMap.get(key));
            }
        } else if (expected instanceof List) {
                List<Object> expectedList = (List<Object>) expected;
                List<Object> actualList = (List<Object>) actual;
                assertEquals(expectedList.size(), actualList.size());
                Iterator<Object> actualIterator = actualList.iterator();
                for (Object element : expectedList) {
                    assertValues(element, actualIterator.next());
                }
        } else {
            assertEquals(expected, actual);
        }
    }

    @Override
    protected Predicate<String> excludePathsFromXContentInsertion() {
        return path -> path.contains(CommonFields.VALUE.getPreferredName());
    }

    @Override
    protected InternalScriptedMetric mutateInstance(InternalScriptedMetric instance) throws IOException {
        String name = instance.getName();
        Object value = instance.aggregation();
        Script reduceScript = instance.reduceScript;
        List<PipelineAggregator> pipelineAggregators = instance.pipelineAggregators();
        Map<String, Object> metaData = instance.getMetaData();
        switch (between(0, 3)) {
        case 0:
            name += randomAlphaOfLength(5);
            break;
        case 1:
            Object newValue = randomValue(valueTypes, 0);
            while ((newValue == null && value == null) || (newValue != null && newValue.equals(value))) {
                int levels = randomIntBetween(1, 3);
                Supplier[] valueTypes = new Supplier[levels];
                for (int i = 0; i < levels; i++) {
                    if (i < levels - 1) {
                        valueTypes[i] = randomFrom(nestedValueSuppliers);
                    } else {
                                                                        valueTypes[i] = randomFrom(leafValueSuppliers);
                    }
                }
                newValue = randomValue(valueTypes, 0);
            }
            value = newValue;
            break;
        case 2:
            reduceScript = new Script(ScriptType.INLINE, MockScriptEngine.NAME, REDUCE_SCRIPT_NAME + "-mutated", Collections.emptyMap());
            break;
        case 3:
            if (metaData == null) {
                metaData = new HashMap<>(1);
            } else {
                metaData = new HashMap<>(instance.getMetaData());
            }
            metaData.put(randomAlphaOfLength(15), randomInt());
            break;
        default:
            throw new AssertionError("Illegal randomisation branch");
        }
        return new InternalScriptedMetric(name, value, reduceScript, pipelineAggregators, metaData);
    }
}
