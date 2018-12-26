

package org.springframework.boot.autoconfigure.freemarker;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.springframework.boot.autoconfigure.template.PathBasedTemplateAvailabilityProvider;
import org.springframework.boot.autoconfigure.template.TemplateAvailabilityProvider;


public class FreeMarkerTemplateAvailabilityProvider
		extends PathBasedTemplateAvailabilityProvider {

	public FreeMarkerTemplateAvailabilityProvider() {
		super("freemarker.template.Configuration",
				FreeMarkerTemplateAvailabilityProperties.class, "spring.freemarker");
	}

	static final class FreeMarkerTemplateAvailabilityProperties
			extends TemplateAvailabilityProperties {

		private List<String> templateLoaderPath = new ArrayList<>(
				Arrays.asList(FreeMarkerProperties.DEFAULT_TEMPLATE_LOADER_PATH));

		FreeMarkerTemplateAvailabilityProperties() {
			super(FreeMarkerProperties.DEFAULT_PREFIX,
					FreeMarkerProperties.DEFAULT_SUFFIX);
		}

		@Override
		protected List<String> getLoaderPath() {
			return this.templateLoaderPath;
		}

		public List<String> getTemplateLoaderPath() {
			return this.templateLoaderPath;
		}

		public void setTemplateLoaderPath(List<String> templateLoaderPath) {
			this.templateLoaderPath = templateLoaderPath;
		}

	}

}
