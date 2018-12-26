

package org.springframework.boot.autoconfigure;

import java.io.IOException;
import java.net.URL;
import java.util.Enumeration;
import java.util.Properties;
import java.util.Set;

import org.springframework.core.io.UrlResource;
import org.springframework.core.io.support.PropertiesLoaderUtils;
import org.springframework.util.StringUtils;


final class AutoConfigurationMetadataLoader {

	protected static final String PATH = "META-INF/"
			+ "spring-autoconfigure-metadata.properties";

	private AutoConfigurationMetadataLoader() {
	}

	public static AutoConfigurationMetadata loadMetadata(ClassLoader classLoader) {
		return loadMetadata(classLoader, PATH);
	}

	static AutoConfigurationMetadata loadMetadata(ClassLoader classLoader, String path) {
		try {
			Enumeration<URL> urls = (classLoader != null ? classLoader.getResources(path)
					: ClassLoader.getSystemResources(path));
			Properties properties = new Properties();
			while (urls.hasMoreElements()) {
				properties.putAll(PropertiesLoaderUtils
						.loadProperties(new UrlResource(urls.nextElement())));
			}
			return loadMetadata(properties);
		}
		catch (IOException ex) {
			throw new IllegalArgumentException(
					"Unable to load @ConditionalOnClass location [" + path + "]", ex);
		}
	}

	static AutoConfigurationMetadata loadMetadata(Properties properties) {
		return new PropertiesAutoConfigurationMetadata(properties);
	}

	
	private static class PropertiesAutoConfigurationMetadata
			implements AutoConfigurationMetadata {

		private final Properties properties;

		PropertiesAutoConfigurationMetadata(Properties properties) {
			this.properties = properties;
		}

		@Override
		public boolean wasProcessed(String className) {
			return this.properties.containsKey(className);
		}

		@Override
		public Integer getInteger(String className, String key) {
			return getInteger(className, key, null);
		}

		@Override
		public Integer getInteger(String className, String key, Integer defaultValue) {
			String value = get(className, key);
			return (value != null ? Integer.valueOf(value) : defaultValue);
		}

		@Override
		public Set<String> getSet(String className, String key) {
			return getSet(className, key, null);
		}

		@Override
		public Set<String> getSet(String className, String key,
				Set<String> defaultValue) {
			String value = get(className, key);
			return (value != null ? StringUtils.commaDelimitedListToSet(value)
					: defaultValue);
		}

		@Override
		public String get(String className, String key) {
			return get(className, key, null);
		}

		@Override
		public String get(String className, String key, String defaultValue) {
			String value = this.properties.getProperty(className + "." + key);
			return (value != null ? value : defaultValue);
		}

	}

}
