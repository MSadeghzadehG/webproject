

package org.springframework.boot.autoconfigure.groovy.template;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.springframework.boot.autoconfigure.template.PathBasedTemplateAvailabilityProvider;
import org.springframework.boot.autoconfigure.template.TemplateAvailabilityProvider;


public class GroovyTemplateAvailabilityProvider
		extends PathBasedTemplateAvailabilityProvider {

	public GroovyTemplateAvailabilityProvider() {
		super("groovy.text.TemplateEngine", GroovyTemplateAvailabilityProperties.class,
				"spring.groovy.template");
	}

	static final class GroovyTemplateAvailabilityProperties
			extends TemplateAvailabilityProperties {

		private List<String> resourceLoaderPath = new ArrayList<>(
				Arrays.asList(GroovyTemplateProperties.DEFAULT_RESOURCE_LOADER_PATH));

		GroovyTemplateAvailabilityProperties() {
			super(GroovyTemplateProperties.DEFAULT_PREFIX,
					GroovyTemplateProperties.DEFAULT_SUFFIX);
		}

		@Override
		protected List<String> getLoaderPath() {
			return this.resourceLoaderPath;
		}

		public List<String> getResourceLoaderPath() {
			return this.resourceLoaderPath;
		}

		public void setResourceLoaderPath(List<String> resourceLoaderPath) {
			this.resourceLoaderPath = resourceLoaderPath;
		}

	}

}
