

package org.springframework.boot.env;

import java.util.Collection;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Objects;
import java.util.stream.StreamSupport;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.json.JsonParser;
import org.springframework.boot.json.JsonParserFactory;
import org.springframework.boot.origin.Origin;
import org.springframework.boot.origin.OriginLookup;
import org.springframework.boot.origin.PropertySourceOrigin;
import org.springframework.core.Ordered;
import org.springframework.core.env.ConfigurableEnvironment;
import org.springframework.core.env.Environment;
import org.springframework.core.env.MapPropertySource;
import org.springframework.core.env.MutablePropertySources;
import org.springframework.core.env.PropertySource;
import org.springframework.core.env.StandardEnvironment;
import org.springframework.util.ClassUtils;
import org.springframework.util.StringUtils;
import org.springframework.web.context.support.StandardServletEnvironment;


public class SpringApplicationJsonEnvironmentPostProcessor
		implements EnvironmentPostProcessor, Ordered {

	
	public static final String SPRING_APPLICATION_JSON_PROPERTY = "spring.application.json";

	
	public static final String SPRING_APPLICATION_JSON_ENVIRONMENT_VARIABLE = "SPRING_APPLICATION_JSON";

	private static final String SERVLET_ENVIRONMENT_CLASS = "org.springframework.web."
			+ "context.support.StandardServletEnvironment";

	
	public static final int DEFAULT_ORDER = Ordered.HIGHEST_PRECEDENCE + 5;

	private static final Log logger = LogFactory
			.getLog(SpringApplicationJsonEnvironmentPostProcessor.class);

	private int order = DEFAULT_ORDER;

	@Override
	public int getOrder() {
		return this.order;
	}

	public void setOrder(int order) {
		this.order = order;
	}

	@Override
	public void postProcessEnvironment(ConfigurableEnvironment environment,
			SpringApplication application) {
		MutablePropertySources propertySources = environment.getPropertySources();
		StreamSupport.stream(propertySources.spliterator(), false)
				.map(JsonPropertyValue::get).filter(Objects::nonNull).findFirst()
				.ifPresent((v) -> processJson(environment, v));
	}

	private void processJson(ConfigurableEnvironment environment,
			JsonPropertyValue propertyValue) {
		try {
			JsonParser parser = JsonParserFactory.getJsonParser();
			Map<String, Object> map = parser.parseMap(propertyValue.getJson());
			if (!map.isEmpty()) {
				addJsonPropertySource(environment,
						new JsonPropertySource(propertyValue, flatten(map)));
			}
		}
		catch (Exception ex) {
			logger.warn("Cannot parse JSON for spring.application.json: "
					+ propertyValue.getJson(), ex);
		}
	}

	
	private Map<String, Object> flatten(Map<String, Object> map) {
		Map<String, Object> result = new LinkedHashMap<>();
		flatten(null, result, map);
		return result;
	}

	private void flatten(String prefix, Map<String, Object> result,
			Map<String, Object> map) {
		prefix = (prefix == null ? "" : prefix + ".");
		for (Map.Entry<String, Object> entry : map.entrySet()) {
			extract(prefix + entry.getKey(), result, entry.getValue());
		}
	}

	@SuppressWarnings("unchecked")
	private void extract(String name, Map<String, Object> result, Object value) {
		if (value instanceof Map) {
			flatten(name, result, (Map<String, Object>) value);
		}
		else if (value instanceof Collection) {
			int index = 0;
			for (Object object : (Collection<Object>) value) {
				extract(name + "[" + index + "]", result, object);
				index++;
			}
		}
		else {
			result.put(name, value);
		}
	}

	private void addJsonPropertySource(ConfigurableEnvironment environment,
			PropertySource<?> source) {
		MutablePropertySources sources = environment.getPropertySources();
		String name = findPropertySource(sources);
		if (sources.contains(name)) {
			sources.addBefore(name, source);
		}
		else {
			sources.addFirst(source);
		}
	}

	private String findPropertySource(MutablePropertySources sources) {
		if (ClassUtils.isPresent(SERVLET_ENVIRONMENT_CLASS, null) && sources
				.contains(StandardServletEnvironment.JNDI_PROPERTY_SOURCE_NAME)) {
			return StandardServletEnvironment.JNDI_PROPERTY_SOURCE_NAME;

		}
		return StandardEnvironment.SYSTEM_PROPERTIES_PROPERTY_SOURCE_NAME;
	}

	private static class JsonPropertySource extends MapPropertySource
			implements OriginLookup<String> {

		private final JsonPropertyValue propertyValue;

		JsonPropertySource(JsonPropertyValue propertyValue, Map<String, Object> source) {
			super(SPRING_APPLICATION_JSON_PROPERTY, source);
			this.propertyValue = propertyValue;
		}

		@Override
		public Origin getOrigin(String key) {
			return this.propertyValue.getOrigin();
		}

	}

	private static class JsonPropertyValue {

		private static final String[] CANDIDATES = { SPRING_APPLICATION_JSON_PROPERTY,
				SPRING_APPLICATION_JSON_ENVIRONMENT_VARIABLE };

		private final PropertySource<?> propertySource;

		private final String propertyName;

		private final String json;

		JsonPropertyValue(PropertySource<?> propertySource, String propertyName,
				String json) {
			this.propertySource = propertySource;
			this.propertyName = propertyName;
			this.json = json;
		}

		public String getJson() {
			return this.json;
		}

		public Origin getOrigin() {
			return PropertySourceOrigin.get(this.propertySource, this.propertyName);
		}

		public static JsonPropertyValue get(PropertySource<?> propertySource) {
			for (String candidate : CANDIDATES) {
				Object value = propertySource.getProperty(candidate);
				if (value != null && value instanceof String
						&& StringUtils.hasLength((String) value)) {
					return new JsonPropertyValue(propertySource, candidate,
							(String) value);
				}
			}
			return null;
		}

	}

}
