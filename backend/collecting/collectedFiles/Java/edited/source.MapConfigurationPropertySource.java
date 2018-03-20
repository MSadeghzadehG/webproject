

package org.springframework.boot.context.properties.source;

import java.util.Collections;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.stream.Stream;

import org.springframework.core.env.MapPropertySource;
import org.springframework.util.Assert;


public class MapConfigurationPropertySource
		implements IterableConfigurationPropertySource {

	private final Map<String, Object> source;

	private final IterableConfigurationPropertySource delegate;

	
	public MapConfigurationPropertySource() {
		this(Collections.emptyMap());
	}

	
	public MapConfigurationPropertySource(Map<?, ?> map) {
		this.source = new LinkedHashMap<>();
		this.delegate = new SpringIterableConfigurationPropertySource(
				new MapPropertySource("source", this.source),
				DefaultPropertyMapper.INSTANCE);
		putAll(map);
	}

	
	public void putAll(Map<?, ?> map) {
		Assert.notNull(map, "Map must not be null");
		assertNotReadOnlySystemAttributesMap(map);
		map.forEach(this::put);
	}

	
	public void put(Object name, Object value) {
		this.source.put((name == null ? null : name.toString()), value);
	}

	@Override
	public Object getUnderlyingSource() {
		return this.source;
	}

	@Override
	public ConfigurationProperty getConfigurationProperty(
			ConfigurationPropertyName name) {
		return this.delegate.getConfigurationProperty(name);
	}

	@Override
	public Iterator<ConfigurationPropertyName> iterator() {
		return this.delegate.iterator();
	}

	@Override
	public Stream<ConfigurationPropertyName> stream() {
		return this.delegate.stream();
	}

	private void assertNotReadOnlySystemAttributesMap(Map<?, ?> map) {
		try {
			map.size();
		}
		catch (UnsupportedOperationException ex) {
			throw new IllegalArgumentException(
					"Security restricted maps are not supported", ex);
		}
	}

}
