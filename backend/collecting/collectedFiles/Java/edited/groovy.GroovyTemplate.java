

package org.springframework.boot.groovy;

import java.io.File;
import java.io.IOException;
import java.io.StringWriter;
import java.net.URL;
import java.util.Collections;
import java.util.Map;

import groovy.lang.Writable;
import groovy.text.GStringTemplateEngine;
import groovy.text.Template;
import groovy.text.TemplateEngine;
import org.codehaus.groovy.control.CompilationFailedException;


public abstract class GroovyTemplate {

	public static String template(String name)
			throws IOException, CompilationFailedException, ClassNotFoundException {
		return template(name, Collections.emptyMap());
	}

	public static String template(String name, Map<String, ?> model)
			throws IOException, CompilationFailedException, ClassNotFoundException {
		return template(new GStringTemplateEngine(), name, model);
	}

	public static String template(TemplateEngine engine, String name,
			Map<String, ?> model)
			throws IOException, CompilationFailedException, ClassNotFoundException {
		Writable writable = getTemplate(engine, name).make(model);
		StringWriter result = new StringWriter();
		writable.writeTo(result);
		return result.toString();
	}

	private static Template getTemplate(TemplateEngine engine, String name)
			throws CompilationFailedException, ClassNotFoundException, IOException {

		File file = new File("templates", name);
		if (file.exists()) {
			return engine.createTemplate(file);
		}

		ClassLoader classLoader = GroovyTemplate.class.getClassLoader();
		URL resource = classLoader.getResource("templates/" + name);
		if (resource != null) {
			return engine.createTemplate(resource);
		}

		return engine.createTemplate(name);
	}

}
