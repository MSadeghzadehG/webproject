

package org.springframework.boot.test.json;

import java.net.URL;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import org.springframework.context.ConfigurableApplicationContext;
import org.springframework.test.context.ContextConfigurationAttributes;
import org.springframework.test.context.ContextCustomizer;
import org.springframework.test.context.ContextCustomizerFactory;
import org.springframework.test.context.MergedContextConfiguration;


class DuplicateJsonObjectContextCustomizerFactory implements ContextCustomizerFactory {

	@Override
	public ContextCustomizer createContextCustomizer(Class<?> testClass,
			List<ContextConfigurationAttributes> configAttributes) {
		return new DuplicateJsonObjectContextCustomizer();
	}

	private static class DuplicateJsonObjectContextCustomizer
			implements ContextCustomizer {

		private final Log logger = LogFactory
				.getLog(DuplicateJsonObjectContextCustomizer.class);

		@Override
		public void customizeContext(ConfigurableApplicationContext context,
				MergedContextConfiguration mergedConfig) {
			List<URL> jsonObjects = findJsonObjects();
			if (jsonObjects.size() > 1) {
				logDuplicateJsonObjectsWarning(jsonObjects);
			}
		}

		private List<URL> findJsonObjects() {
			List<URL> jsonObjects = new ArrayList<>();
			try {
				Enumeration<URL> resources = getClass().getClassLoader()
						.getResources("org/json/JSONObject.class");
				while (resources.hasMoreElements()) {
					jsonObjects.add(resources.nextElement());
				}
			}
			catch (Exception ex) {
							}
			return jsonObjects;
		}

		private void logDuplicateJsonObjectsWarning(List<URL> jsonObjects) {
			StringBuilder message = new StringBuilder(
					String.format("%n%nFound multiple occurrences of"
							+ " org.json.JSONObject on the class path:%n%n"));
			for (URL jsonObject : jsonObjects) {
				message.append(String.format("\t%s%n", jsonObject));
			}
			message.append(String.format("%nYou may wish to exclude one of them to ensure"
					+ " predictable runtime behavior%n"));
			this.logger.warn(message);
		}

		@Override
		public int hashCode() {
			return getClass().hashCode();
		}

		@Override
		public boolean equals(Object obj) {
			if (obj == null || obj.getClass() != getClass()) {
				return false;
			}
			return true;
		}

	}

}
