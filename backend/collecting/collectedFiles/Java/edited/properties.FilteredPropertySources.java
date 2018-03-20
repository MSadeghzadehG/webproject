

package org.springframework.boot.context.properties;

import java.util.Arrays;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;
import java.util.stream.StreamSupport;

import org.springframework.core.env.PropertySource;
import org.springframework.core.env.PropertySources;


final class FilteredPropertySources implements PropertySources {

	private final PropertySources delegate;

	private final Set<String> filtered;

	FilteredPropertySources(PropertySources delegate, String... filtered) {
		this.delegate = delegate;
		this.filtered = new HashSet<>(Arrays.asList(filtered));
	}

	@Override
	public boolean contains(String name) {
		if (included(name)) {
			return this.delegate.contains(name);
		}
		return false;
	}

	@Override
	public PropertySource<?> get(String name) {
		if (included(name)) {
			return this.delegate.get(name);
		}
		return null;
	}

	@Override
	public Iterator<PropertySource<?>> iterator() {
		return StreamSupport.stream(this.delegate.spliterator(), false)
				.filter(this::included).iterator();
	}

	private boolean included(PropertySource<?> propertySource) {
		return included(propertySource.getName());
	}

	private boolean included(String name) {
		return (!this.filtered.contains(name));
	}

}
