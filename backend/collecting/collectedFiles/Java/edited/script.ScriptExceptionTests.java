

package org.elasticsearch.script;

import org.elasticsearch.common.io.stream.DataOutputStreamOutput;
import org.elasticsearch.common.io.stream.InputStreamStreamInput;
import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.common.io.stream.StreamOutput;
import org.elasticsearch.test.ESTestCase;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;


public class ScriptExceptionTests extends ESTestCase {
    
    
    public void testRoundTrip() throws IOException {
        ScriptException e = new ScriptException("messageData", new Exception("causeData"), Arrays.asList("stack1", "stack2"), 
                                                "sourceData", "langData");
        
        ByteArrayOutputStream bytes = new ByteArrayOutputStream();
        StreamOutput output = new DataOutputStreamOutput(new DataOutputStream(bytes));
        e.writeTo(output);
        output.close();
        
        StreamInput input = new InputStreamStreamInput(new ByteArrayInputStream(bytes.toByteArray()));
        ScriptException e2 = new ScriptException(input);
        input.close();
        
        assertEquals(e.getMessage(), e2.getMessage());
        assertEquals(e.getScriptStack(), e2.getScriptStack());
        assertEquals(e.getScript(), e2.getScript());
        assertEquals(e.getLang(), e2.getLang());
    }
    
    
    public void testJsonOutput() {
        ScriptException e = new ScriptException("messageData", new Exception("causeData"), Arrays.asList("stack1", "stack2"), 
                                                "sourceData", "langData");
        String json = e.toJsonString();
        assertTrue(json.contains(e.getMessage()));
        assertTrue(json.contains(e.getCause().getMessage()));
        assertTrue(json.contains("stack1"));
        assertTrue(json.contains("stack2"));
        assertTrue(json.contains(e.getScript()));
        assertTrue(json.contains(e.getLang()));
    }

    
    public void testImmutableStack() {
        ScriptException e = new ScriptException("a", new Exception(), Arrays.asList("element1", "element2"), "a", "b");
        List<String> stack = e.getScriptStack();
        expectThrows(UnsupportedOperationException.class, () -> {
            stack.add("no");
        });
    }

    
    public void testNoLeniency() {
        expectThrows(NullPointerException.class, () -> {
           new ScriptException(null, new Exception(), Collections.emptyList(), "a", "b");
        });
        expectThrows(NullPointerException.class, () -> {
            new ScriptException("test", null, Collections.emptyList(), "a", "b");
        });
        expectThrows(NullPointerException.class, () -> {
            new ScriptException("test", new Exception(), null, "a", "b");
        });
        expectThrows(NullPointerException.class, () -> {
            new ScriptException("test", new Exception(), Collections.emptyList(), null, "b");
        });
        expectThrows(NullPointerException.class, () -> {
            new ScriptException("test", new Exception(), Collections.emptyList(), "a", null);
        });
    }
}
