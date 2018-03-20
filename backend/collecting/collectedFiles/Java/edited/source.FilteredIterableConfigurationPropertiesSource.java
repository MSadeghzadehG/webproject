

package org.springframework.boot.context.properties.source;

import java.util.function.Predicate;
import java.util.stream.Stream;
import java.util.stream.StreamSupport;


class FilteredIterableConfigurationPropertiesSource
		extends FilteredConfigurationPropertiesSource
		implements IterableConfigurationPropertySource {

	FilteredIterableConfigurationPropertiesSource(
			IterableConfigurationPropertySource source,
			Predicate<ConfigurationPropertyName> filter) {
		super(source, filter);
	}

	@Override
	public Stream<ConfigurationPropertyName> stream() {
		return StreamSupport.stream(getSource().spliterator(), false).filter(getFilter());
	}

	@Override
	protected IterableConfigurationPropertySource getSource() {
		return (IterableConfigurationPropertySource) super.getSource();
	}

	@Override
	public ConfigurationPropertyState containsDescendantOf(
			ConfigurationPropertyName name) {
		return ConfigurationPropertyState.search(this, name::isAncestorOf);
	}

}
