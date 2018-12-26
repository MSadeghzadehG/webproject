

package org.springframework.boot.test.util;

import java.io.Closeable;
import java.util.Arrays;
import java.util.Collections;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Objects;
import java.util.concurrent.Callable;
import java.util.stream.Stream;
import java.util.stream.StreamSupport;

import org.springframework.boot.context.properties.source.ConfigurationPropertySources;
import org.springframework.context.ApplicationContext;
import org.springframework.context.ConfigurableApplicationContext;
import org.springframework.core.env.ConfigurableEnvironment;
import org.springframework.core.env.Environment;
import org.springframework.core.env.MapPropertySource;
import org.springframework.core.env.MutablePropertySources;
import org.springframework.core.env.PropertySource;
import org.springframework.core.env.StandardEnvironment;
import org.springframework.core.env.SystemEnvironmentPropertySource;
import org.springframework.util.Assert;
import org.springframework.util.StringUtils;


public final class TestPropertyValues {

	private static final TestPropertyValues EMPTY = new TestPropertyValues(
			Collections.emptyMap());

	private final Map<String, Object> properties;

	private TestPropertyValues(Map<String, Object> properties) {
		this.properties = Collections.unmodifiableMap(properties);
	}

	
	public TestPropertyValues and(String... pairs) {
		return and(Arrays.stream(pairs).map(Pair::parse));
	}

	private TestPropertyValues and(Stream<Pair> pairs) {
		Map<String, Object> properties = new LinkedHashMap<>(this.properties);
		pairs.filter(Objects::nonNull).forEach((pair) -> pair.addTo(properties));
		return new TestPropertyValues(properties);
	}

	
	public void applyTo(ConfigurableApplicationContext context) {
		applyTo(context.getEnvironment());
	}

	
	public void applyTo(ConfigurableEnvironment environment) {
		applyTo(environment, Type.MAP);
	}

	
	public void applyTo(ConfigurableEnvironment environment, Type type) {
		applyTo(environment, type, type.applySuffix("test"));
	}

	
	public void applyTo(ConfigurableEnvironment environment, Type type, String name) {
		Assert.notNull(environment, "Environment must not be null");
		Assert.notNull(type, "Property source type must not be null");
		Assert.notNull(name, "Property source name must not be null");
		MutablePropertySources sources = environment.getPropertySources();
		addToSources(sources, type, name);
		ConfigurationPropertySources.attach(environment);
	}

	
	public <T> T applyToSystemProperties(Callable<T> call) {
		try (SystemPropertiesHandler handler = new SystemPropertiesHandler()) {
			return call.call();
		}
		catch (Exception ex) {
			rethrow(ex);
			throw new IllegalStateException("Original cause not rethrown", ex);
		}
	}

	@SuppressWarnings("unchecked")
	private <E extends Throwable> void rethrow(Throwable e) throws E {
		throw (E) e;
	}

	@SuppressWarnings("unchecked")
	private void addToSources(MutablePropertySources sources, Type type, String name) {
		if (sources.contains(name)) {
			PropertySource<?> propertySource = sources.get(name);
			if (propertySource.getClass() == type.getSourceClass()) {
				((Map<String, Object>) propertySource.getSource())
						.putAll(this.properties);
				return;
			}
		}
		Map<String, Object> source = new LinkedHashMap<>(this.properties);
		sources.addFirst((type.equals(Type.MAP) ? new MapPropertySource(name, source)
				: new SystemEnvironmentPropertySource(name, source)));
	}

	
	public static TestPropertyValues of(String... pairs) {
		return of(Stream.of(pairs));
	}

	
	public static TestPropertyValues of(Iterable<String> pairs) {
		if (pairs == null) {
			return empty();
		}
		return of(StreamSupport.stream(pairs.spliterator(), false));
	}

	
	public static TestPropertyValues of(Stream<String> pairs) {
		if (pairs == null) {
			return empty();
		}
		return empty().and(pairs.map(Pair::parse));
	}

	
	public static TestPropertyValues empty() {
		return EMPTY;
	}

	
	public enum Type {

		
		SYSTEM_ENVIRONMENT(SystemEnvironmentPropertySource.class,
				StandardEnvironment.SYSTEM_ENVIRONMENT_PROPERTY_SOURCE_NAME),

		
		MAP(MapPropertySource.class, null);

		private final Class<? extends MapPropertySource> sourceClass;

		private final String suffix;

		Type(Class<? extends MapPropertySource> sourceClass, String suffix) {
			this.sourceClass = sourceClass;
			this.suffix = suffix;
		}

		public Class<? extends MapPropertySource> getSourceClass() {
			return this.sourceClass;
		}

		protected String applySuffix(String name) {
			return (this.suffix == null ? name : name + "-" + this.suffix);
		}

	}

	
	public static class Pair {

		private String name;

		private String value;

		public Pair(String name, String value) {
			Assert.hasLength(name, "Name must not be empty");
			this.name = name;
			this.value = value;
		}

		public void addTo(Map<String, Object> properties) {
			properties.put(this.name, this.value);
		}

		public static Pair parse(String pair) {
			int index = getSeparatorIndex(pair);
			String name = (index > 0 ? pair.substring(0, index) : pair);
			String value = (index > 0 ? pair.substring(index + 1) : "");
			return of(name.trim(), value.trim());
		}

		private static int getSeparatorIndex(String pair) {
			int colonIndex = pair.indexOf(':');
			int equalIndex = pair.indexOf('=');
			if (colonIndex == -1) {
				return equalIndex;
			}
			if (equalIndex == -1) {
				return colonIndex;
			}
			return Math.min(colonIndex, equalIndex);
		}

		private static Pair of(String name, String value) {
			if (StringUtils.isEmpty(name) && StringUtils.isEmpty(value)) {
				return null;
			}
			return new Pair(name, value);
		}

	}

	
	private class SystemPropertiesHandler implements Closeable {

		private final Map<String, String> previous;

		SystemPropertiesHandler() {
			this.previous = apply(TestPropertyValues.this.properties);
		}

		private Map<String, String> apply(Map<String, ?> properties) {
			Map<String, String> previous = new LinkedHashMap<>();
			properties.forEach((name, value) -> previous.put(name,
					setOrClear(name, (String) value)));
			return previous;
		}

		@Override
		public void close() {
			this.previous.forEach(this::setOrClear);
		}

		private String setOrClear(String name, String value) {
			Assert.notNull(name, "Name must not be null");
			if (StringUtils.isEmpty(value)) {
				return (String) System.getProperties().remove(name);
			}
			return (String) System.getProperties().setProperty(name, value);
		}

	}

}
