

package org.elasticsearch.painless;

import org.apache.lucene.index.LeafReaderContext;
import org.elasticsearch.script.ExecutableScript;
import org.elasticsearch.script.SearchScript;
import org.elasticsearch.search.lookup.LeafSearchLookup;
import org.elasticsearch.search.lookup.SearchLookup;

import java.util.HashMap;
import java.util.Map;
import java.util.function.DoubleSupplier;
import java.util.function.Function;


final class ScriptImpl extends SearchScript {

    
    private final GenericElasticsearchScript script;

    
    private final Map<String, Object> variables;

    
    private final DoubleSupplier scoreLookup;

    
    private final Function<Map<String, Object>, Map<?, ?>> ctxLookup;

    
    private Object aggregationValue;

    
    ScriptImpl(GenericElasticsearchScript script, Map<String, Object> vars, SearchLookup lookup, LeafReaderContext leafContext) {
        super(null, lookup, leafContext);
        this.script = script;
        this.variables = new HashMap<>();

        if (vars != null) {
            variables.putAll(vars);
        }
        LeafSearchLookup leafLookup = getLeafLookup();
        if (leafLookup != null) {
            variables.putAll(leafLookup.asMap());
        }

        scoreLookup = script.needs_score() ? this::getScore : () -> 0.0;
        ctxLookup = script.needsCtx() ? variables -> (Map<?, ?>) variables.get("ctx") : variables -> null;
    }

    @Override
    public Map<String, Object> getParams() {
        return variables;
    }

    @Override
    public void setNextVar(final String name, final Object value) {
        variables.put(name, value);
    }

    @Override
    public void setNextAggregationValue(Object value) {
        this.aggregationValue = value;
    }

    @Override
    public Object run() {
        return script.execute(variables, scoreLookup.getAsDouble(), getDoc(), aggregationValue, ctxLookup.apply(variables));
    }

    @Override
    public double runAsDouble() {
        return ((Number)run()).doubleValue();
    }

    @Override
    public long runAsLong() {
        return ((Number)run()).longValue();
    }
}
