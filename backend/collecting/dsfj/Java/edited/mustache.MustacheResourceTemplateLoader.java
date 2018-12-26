

package org.springframework.boot.autoconfigure.mustache;

import java.io.InputStreamReader;
import java.io.Reader;

import com.samskivert.mustache.Mustache;
import com.samskivert.mustache.Mustache.Compiler;
import com.samskivert.mustache.Mustache.TemplateLoader;

import org.springframework.context.ResourceLoaderAware;
import org.springframework.core.io.DefaultResourceLoader;
import org.springframework.core.io.Resource;
import org.springframework.core.io.ResourceLoader;


public class MustacheResourceTemplateLoader
		implements TemplateLoader, ResourceLoaderAware {

	private String prefix = "";

	private String suffix = "";

	private String charSet = "UTF-8";

	private ResourceLoader resourceLoader = new DefaultResourceLoader();

	public MustacheResourceTemplateLoader() {
	}

	public MustacheResourceTemplateLoader(String prefix, String suffix) {
		this.prefix = prefix;
		this.suffix = suffix;
	}

	
	public void setCharset(String charSet) {
		this.charSet = charSet;
	}

	
	@Override
	public void setResourceLoader(ResourceLoader resourceLoader) {
		this.resourceLoader = resourceLoader;
	}

	@Override
	public Reader getTemplate(String name) throws Exception {
		return new InputStreamReader(this.resourceLoader
				.getResource(this.prefix + name + this.suffix).getInputStream(),
				this.charSet);
	}

}
