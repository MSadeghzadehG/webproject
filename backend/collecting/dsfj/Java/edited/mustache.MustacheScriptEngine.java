
package org.elasticsearch.script.mustache;

import com.github.mustachejava.Mustache;
import com.github.mustachejava.MustacheFactory;

import java.io.StringReader;
import org.apache.logging.log4j.Logger;
import org.apache.logging.log4j.message.ParameterizedMessage;
import org.apache.logging.log4j.util.Supplier;
import org.elasticsearch.SpecialPermission;
import org.elasticsearch.common.logging.ESLoggerFactory;
import org.elasticsearch.script.GeneralScriptException;
import org.elasticsearch.script.Script;
import org.elasticsearch.script.ScriptContext;
import org.elasticsearch.script.ScriptEngine;
import org.elasticsearch.script.TemplateScript;

import java.io.Reader;
import java.io.StringWriter;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.util.Map;


public final class MustacheScriptEngine implements ScriptEngine {
    private static final Logger logger = ESLoggerFactory.getLogger(MustacheScriptEngine.class);

    public static final String NAME = "mustache";

    
    @Override
    public <T> T compile(String templateName, String templateSource, ScriptContext<T> context, Map<String, String> options) {
        if (context.instanceClazz.equals(TemplateScript.class) == false) {
            throw new IllegalArgumentException("mustache engine does not know how to handle context [" + context.name + "]");
        }
        final MustacheFactory factory = createMustacheFactory(options);
        Reader reader = new StringReader(templateSource);
        Mustache template = factory.compile(reader, "query-template");
        TemplateScript.Factory compiled = params -> new MustacheExecutableScript(template, params);
        return context.factoryClazz.cast(compiled);
    }

    private CustomMustacheFactory createMustacheFactory(Map<String, String> options) {
        if (options == null || options.isEmpty() || options.containsKey(Script.CONTENT_TYPE_OPTION) == false) {
            return new CustomMustacheFactory();
        }
        return new CustomMustacheFactory(options.get(Script.CONTENT_TYPE_OPTION));
    }

    @Override
    public String getType() {
        return NAME;
    }

    
    private class MustacheExecutableScript extends TemplateScript {
        
        private Mustache template;

        private Map<String, Object> params;

        
        MustacheExecutableScript(Mustache template, Map<String, Object> params) {
            super(params);
            this.template = template;
            this.params = params;
        }

        @Override
        public String execute() {
            final StringWriter writer = new StringWriter();
            try {
                                SpecialPermission.check();
                AccessController.doPrivileged((PrivilegedAction<Void>) () -> {
                    template.execute(writer, params);
                    return null;
                });
            } catch (Exception e) {
                logger.error((Supplier<?>) () -> new ParameterizedMessage("Error running {}", template), e);
                throw new GeneralScriptException("Error running " + template, e);
            }
            return writer.toString();
        }
    }
}
