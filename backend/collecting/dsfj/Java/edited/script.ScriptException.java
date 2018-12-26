package org.elasticsearch.script;



import java.io.IOException;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Objects;

import org.elasticsearch.ElasticsearchException;
import org.elasticsearch.common.Strings;
import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.common.io.stream.StreamOutput;
import org.elasticsearch.common.xcontent.ToXContent;
import org.elasticsearch.common.xcontent.XContentBuilder;
import org.elasticsearch.common.xcontent.XContentFactory;


@SuppressWarnings("serial")
public class ScriptException extends ElasticsearchException {
    private final List<String> scriptStack;
    private final String script;
    private final String lang;

    
    public ScriptException(String message, Throwable cause, List<String> scriptStack, String script, String lang) {
        super(Objects.requireNonNull(message), Objects.requireNonNull(cause));
        this.scriptStack = Collections.unmodifiableList(Objects.requireNonNull(scriptStack));
        this.script = Objects.requireNonNull(script);
        this.lang = Objects.requireNonNull(lang);
    }

    
    public ScriptException(StreamInput in) throws IOException {
        super(in);
        scriptStack = Arrays.asList(in.readStringArray());
        script = in.readString();
        lang = in.readString();
    }

    @Override
    public void writeTo(StreamOutput out) throws IOException {
        super.writeTo(out);
        out.writeStringArray(scriptStack.toArray(new String[0]));
        out.writeString(script);
        out.writeString(lang);
    }

    @Override
    protected void metadataToXContent(XContentBuilder builder, Params params) throws IOException {
        builder.field("script_stack", scriptStack);
        builder.field("script", script);
        builder.field("lang", lang);
    }

    
    public List<String> getScriptStack() {
        return scriptStack;
    }

    
    public String getScript() {
        return script;
    }

    
    public String getLang() {
        return lang;
    }

    
    public String toJsonString() {
        try {
            XContentBuilder json = XContentFactory.jsonBuilder().prettyPrint();
            json.startObject();
            toXContent(json, ToXContent.EMPTY_PARAMS);
            json.endObject();
            return Strings.toString(json);
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }
}
