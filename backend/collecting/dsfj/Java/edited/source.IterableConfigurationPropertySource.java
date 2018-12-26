

package org.springframework.boot.context.properties.source;

import java.util.Iterator;
import java.util.function.Predicate;
import java.util.stream.Stream;

import org.springframework.boot.origin.OriginTrackedValue;


public interface IterableConfigurationPropertySource
		extends ConfigurationPropertySource, Iterable<ConfigurationPropertyName> {

	
	@Override
	default Iterator<ConfigurationPropertyName> iterator() {
		return stream().iterator();
	}

	
	Stream<ConfigurationPropertyName> stream();

	@Override
	default ConfigurationPropertyState containsDescendantOf(
			ConfigurationPropertyName name) {
		return ConfigurationPropertyState.search(this, name::isAncestorOf);
	}

	@Override
	default IterableConfigurationPropertySource filter(
			Predicate<ConfigurationPropertyName> filter) {
		return new FilteredIterableConfigurationPropertiesSource(this, filter);
	}

	@Override
	default IterableConfigurationPropertySource withAliases(
			ConfigurationPropertyNameAliases aliases) {
		return new AliasedIterableConfigurationPropertySource(this, aliases);
	}

}
