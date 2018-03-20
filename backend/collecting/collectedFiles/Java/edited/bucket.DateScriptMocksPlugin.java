

package org.elasticsearch.search.aggregations.bucket;

import org.elasticsearch.script.MockScriptPlugin;
import org.elasticsearch.search.lookup.LeafDocLookup;
import org.joda.time.DateTime;
import org.joda.time.DateTimeZone;

import java.util.HashMap;
import java.util.Map;
import java.util.function.Function;


public class DateScriptMocksPlugin extends MockScriptPlugin {
    static final String EXTRACT_FIELD = "extract_field";
    static final String DOUBLE_PLUS_ONE_MONTH = "double_date_plus_1_month";
    static final String LONG_PLUS_ONE_MONTH = "long_date_plus_1_month";

    @Override
    public Map<String, Function<Map<String, Object>, Object>> pluginScripts() {
        Map<String, Function<Map<String, Object>, Object>> scripts = new HashMap<>();
        scripts.put(EXTRACT_FIELD, params -> {
            LeafDocLookup docLookup = (LeafDocLookup) params.get("doc");
            String fieldname = (String) params.get("fieldname");
            return docLookup.get(fieldname);
        });
        scripts.put(DOUBLE_PLUS_ONE_MONTH, params ->
            new DateTime(Double.valueOf((double) params.get("_value")).longValue(), DateTimeZone.UTC).plusMonths(1).getMillis());
        scripts.put(LONG_PLUS_ONE_MONTH, params ->
            new DateTime((long) params.get("_value"), DateTimeZone.UTC).plusMonths(1).getMillis());
        return scripts;
    }
}
