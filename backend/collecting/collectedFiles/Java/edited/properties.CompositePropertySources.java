

package org.springframework.boot.context.properties;

import java.util.Arrays;
import java.util.Iterator;
import java.util.List;
import java.util.stream.Collectors;
import java.util.stream.StreamSupport;

import org.springframework.core.env.PropertySource;
import org.springframework.core.env.PropertySources;


final class CompositePropertySources implements PropertySources {

	private final List<PropertySources> propertySources;

	CompositePropertySources(PropertySources... propertySources) {
		this.propertySources = Arrays.asList(propertySources);
	}

	@Override
	public Iterator<PropertySource<?>> iterator() {
		return this.propertySources.stream()
				.flatMap((sources) -> StreamSupport.stream(sources.spliterator(), false))
				.collect(Collectors.toList()).iterator();
	}

	@Override
	public boolean contains(String name) {
		for (PropertySources sources : this.propertySources) {
			if (sources.contains(name)) {
				return true;
			}
		}
		return false;
	}

	@Override
	public PropertySource<?> get(String name) {
		for (PropertySources sources : this.propertySources) {
			PropertySource<?> source = sources.get(name);
			if (source != null) {
				return source;
			}
		}
		return null;
	}

}
