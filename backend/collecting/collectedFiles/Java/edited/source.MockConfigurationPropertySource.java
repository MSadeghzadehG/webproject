

package org.springframework.boot.context.properties.source;

import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.stream.Stream;

import org.springframework.boot.origin.MockOrigin;
import org.springframework.boot.origin.OriginTrackedValue;


public class MockConfigurationPropertySource
		implements IterableConfigurationPropertySource {

	private final Map<ConfigurationPropertyName, OriginTrackedValue> map = new LinkedHashMap<>();

	public MockConfigurationPropertySource() {
	}

	public MockConfigurationPropertySource(String configurationPropertyName,
			Object value) {
		this(configurationPropertyName, value, null);
	}

	public MockConfigurationPropertySource(String configurationPropertyName, Object value,
			String origin) {
		put(ConfigurationPropertyName.of(configurationPropertyName),
				OriginTrackedValue.of(value, MockOrigin.of(origin)));
	}

	public void put(String name, String value) {
		put(ConfigurationPropertyName.of(name), value);
	}

	public void put(ConfigurationPropertyName name, String value) {
		put(name, OriginTrackedValue.of(value));
	}

	private void put(ConfigurationPropertyName name, OriginTrackedValue value) {
		this.map.put(name, value);
	}

	public ConfigurationPropertySource nonIterable() {
		return new NonIterable();
	}

	@Override
	public Iterator<ConfigurationPropertyName> iterator() {
		return this.map.keySet().iterator();
	}

	@Override
	public Stream<ConfigurationPropertyName> stream() {
		return this.map.keySet().stream();
	}

	@Override
	public Object getUnderlyingSource() {
		return this.map;
	}

	@Override
	public ConfigurationProperty getConfigurationProperty(
			ConfigurationPropertyName name) {
		OriginTrackedValue result = this.map.get(name);
		if (result == null) {
			result = findValue(name);
		}
		return ConfigurationProperty.of(name, result);
	}

	private OriginTrackedValue findValue(ConfigurationPropertyName name) {
		return this.map.get(name);
	}

	private class NonIterable implements ConfigurationPropertySource {

		@Override
		public Object getUnderlyingSource() {
			return MockConfigurationPropertySource.this.map;
		}

		@Override
		public ConfigurationProperty getConfigurationProperty(
				ConfigurationPropertyName name) {
			return MockConfigurationPropertySource.this.getConfigurationProperty(name);
		}

		@Override
		public ConfigurationPropertyState containsDescendantOf(
				ConfigurationPropertyName name) {
			return ConfigurationPropertyState.UNKNOWN;
		}

	}

}
