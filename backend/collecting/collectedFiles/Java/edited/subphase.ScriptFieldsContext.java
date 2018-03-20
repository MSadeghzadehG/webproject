

package org.elasticsearch.search.fetch.subphase;

import org.elasticsearch.script.SearchScript;

import java.util.ArrayList;
import java.util.List;

public class ScriptFieldsContext {

    public static class ScriptField {
        private final String name;
        private final SearchScript.LeafFactory script;
        private final boolean ignoreException;

        public ScriptField(String name, SearchScript.LeafFactory script, boolean ignoreException) {
            this.name = name;
            this.script = script;
            this.ignoreException = ignoreException;
        }

        public String name() {
            return name;
        }

        public SearchScript.LeafFactory script() {
            return this.script;
        }

        public boolean ignoreException() {
            return ignoreException;
        }
    }

    private List<ScriptField> fields = new ArrayList<>();

    public ScriptFieldsContext() {
    }

    public void add(ScriptField field) {
        this.fields.add(field);
    }

    public List<ScriptField> fields() {
        return this.fields;
    }
}
