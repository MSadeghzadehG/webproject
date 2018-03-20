

package org.elasticsearch.example.expertscript;

import java.io.IOException;
import java.io.UncheckedIOException;
import java.util.Collection;
import java.util.Map;

import org.apache.lucene.index.LeafReaderContext;
import org.apache.lucene.index.PostingsEnum;
import org.apache.lucene.index.Term;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.plugins.Plugin;
import org.elasticsearch.plugins.ScriptPlugin;
import org.elasticsearch.script.ScriptContext;
import org.elasticsearch.script.ScriptEngine;
import org.elasticsearch.script.SearchScript;


public class ExpertScriptPlugin extends Plugin implements ScriptPlugin {

    @Override
    public ScriptEngine getScriptEngine(Settings settings, Collection<ScriptContext<?>> contexts) {
        return new MyExpertScriptEngine();
    }

    
        private static class MyExpertScriptEngine implements ScriptEngine {
        @Override
        public String getType() {
            return "expert_scripts";
        }

        @Override
        public <T> T compile(String scriptName, String scriptSource, ScriptContext<T> context, Map<String, String> params) {
            if (context.equals(SearchScript.CONTEXT) == false) {
                throw new IllegalArgumentException(getType() + " scripts cannot be used for context [" + context.name + "]");
            }
                        if ("pure_df".equals(scriptSource)) {
                SearchScript.Factory factory = (p, lookup) -> new SearchScript.LeafFactory() {
                    final String field;
                    final String term;
                    {
                        if (p.containsKey("field") == false) {
                            throw new IllegalArgumentException("Missing parameter [field]");
                        }
                        if (p.containsKey("term") == false) {
                            throw new IllegalArgumentException("Missing parameter [term]");
                        }
                        field = p.get("field").toString();
                        term = p.get("term").toString();
                    }

                    @Override
                    public SearchScript newInstance(LeafReaderContext context) throws IOException {
                        PostingsEnum postings = context.reader().postings(new Term(field, term));
                        if (postings == null) {
                                                        return new SearchScript(p, lookup, context) {
                                @Override
                                public double runAsDouble() {
                                    return 0.0d;
                                }
                            };
                        }
                        return new SearchScript(p, lookup, context) {
                            int currentDocid = -1;
                            @Override
                            public void setDocument(int docid) {
                                                                if (postings.docID() < docid) {
                                    try {
                                        postings.advance(docid);
                                    } catch (IOException e) {
                                        throw new UncheckedIOException(e);
                                    }
                                }
                                currentDocid = docid;
                            }
                            @Override
                            public double runAsDouble() {
                                if (postings.docID() != currentDocid) {
                                                                        return 0.0d;
                                }
                                try {
                                    return postings.freq();
                                } catch (IOException e) {
                                    throw new UncheckedIOException(e);
                                }
                            }
                        };
                    }

                    @Override
                    public boolean needs_score() {
                        return false;
                    }
                };
                return context.factoryClazz.cast(factory);
            }
            throw new IllegalArgumentException("Unknown script name " + scriptSource);
        }

        @Override
        public void close() {
                    }
    }
    }
