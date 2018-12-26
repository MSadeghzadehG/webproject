

package org.elasticsearch.ingest.common;

import org.elasticsearch.ingest.AbstractProcessor;
import org.elasticsearch.ingest.ConfigurationUtils;
import org.elasticsearch.ingest.IngestDocument;
import org.elasticsearch.ingest.Processor;
import org.elasticsearch.script.ScriptService;
import org.elasticsearch.script.TemplateScript;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;


public final class RemoveProcessor extends AbstractProcessor {

    public static final String TYPE = "remove";

    private final List<TemplateScript.Factory> fields;

    RemoveProcessor(String tag, List<TemplateScript.Factory> fields) {
        super(tag);
        this.fields = new ArrayList<>(fields);
    }

    public List<TemplateScript.Factory> getFields() {
        return fields;
    }

    @Override
    public void execute(IngestDocument document) {
       fields.forEach(document::removeField);
    }

    @Override
    public String getType() {
        return TYPE;
    }

    public static final class Factory implements Processor.Factory {

        private final ScriptService scriptService;

        public Factory(ScriptService scriptService) {
            this.scriptService = scriptService;
        }

        @Override
        public RemoveProcessor create(Map<String, Processor.Factory> registry, String processorTag,
                                      Map<String, Object> config) throws Exception {
            final List<String> fields = new ArrayList<>();
            final Object field = ConfigurationUtils.readObject(TYPE, processorTag, config, "field");
            if (field instanceof List) {
                fields.addAll((List) field);
            } else {
                fields.add((String) field);
            }

            final List<TemplateScript.Factory> compiledTemplates = fields.stream()
                .map(f -> ConfigurationUtils.compileTemplate(TYPE, processorTag, "field", f, scriptService))
                .collect(Collectors.toList());
            return new RemoveProcessor(processorTag, compiledTemplates);
        }
    }
}

