

package org.elasticsearch.painless;

import org.elasticsearch.ElasticsearchException;
import org.elasticsearch.common.io.stream.BytesStreamOutput;
import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.painless.spi.Whitelist;
import org.elasticsearch.script.ScriptException;

import java.io.IOException;
import java.util.Map;

import static java.util.Collections.singletonList;
import static java.util.Collections.singletonMap;
import static org.hamcrest.Matchers.hasEntry;
import static org.hamcrest.Matchers.hasKey;
import static org.hamcrest.Matchers.not;

public class DebugTests extends ScriptTestCase {
    private final Definition definition = new Definition(Whitelist.BASE_WHITELISTS);

    public void testExplain() {
                Object dummy = new Object();
        PainlessExplainError e = expectScriptThrows(PainlessExplainError.class, () -> exec(
                "Debug.explain(params.a)", singletonMap("a", dummy), true));
        assertSame(dummy, e.getObjectToExplain());
        assertThat(e.getHeaders(definition), hasEntry("es.to_string", singletonList(dummy.toString())));
        assertThat(e.getHeaders(definition), hasEntry("es.java_class", singletonList("java.lang.Object")));
        assertThat(e.getHeaders(definition), hasEntry("es.painless_class", singletonList("java.lang.Object")));

                e = expectScriptThrows(PainlessExplainError.class, () -> exec("Debug.explain(null)"));
        assertNull(e.getObjectToExplain());
        assertThat(e.getHeaders(definition), hasEntry("es.to_string", singletonList("null")));
        assertThat(e.getHeaders(definition), not(hasKey("es.java_class")));
        assertThat(e.getHeaders(definition), not(hasKey("es.painless_class")));

                e = expectScriptThrows(PainlessExplainError.class, () -> exec(
                "try {\n"
              + "  Debug.explain(params.a)\n"
              + "} catch (Exception e) {\n"
              + "  return 1\n"
              + "}", singletonMap("a", dummy), true));
        assertSame(dummy, e.getObjectToExplain());
    }

    
    public void testPainlessExplainErrorSerialization() throws IOException {
        Map<String, Object> params = singletonMap("a", "jumped over the moon");
        ScriptException e = expectThrows(ScriptException.class, () -> exec("Debug.explain(params.a)", params, true));
        assertEquals(singletonList("jumped over the moon"), e.getMetadata("es.to_string"));
        assertEquals(singletonList("java.lang.String"), e.getMetadata("es.java_class"));
        assertEquals(singletonList("java.lang.String"), e.getMetadata("es.painless_class"));

        try (BytesStreamOutput out = new BytesStreamOutput()) {
            out.writeException(e);
            try (StreamInput in = out.bytes().streamInput()) {
                ElasticsearchException read = (ScriptException) in.readException();
                assertEquals(singletonList("jumped over the moon"), read.getMetadata("es.to_string"));
                assertEquals(singletonList("java.lang.String"), read.getMetadata("es.java_class"));
                assertEquals(singletonList("java.lang.String"), read.getMetadata("es.painless_class"));
            }
        }
    }
}
