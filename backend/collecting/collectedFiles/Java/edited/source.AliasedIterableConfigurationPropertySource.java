

package org.springframework.boot.context.properties.source;

import java.util.List;
import java.util.stream.Stream;
import java.util.stream.StreamSupport;

import org.springframework.util.CollectionUtils;


class AliasedIterableConfigurationPropertySource
		extends AliasedConfigurationPropertySource
		implements IterableConfigurationPropertySource {

	AliasedIterableConfigurationPropertySource(IterableConfigurationPropertySource source,
			ConfigurationPropertyNameAliases aliases) {
		super(source, aliases);
	}

	@Override
	public Stream<ConfigurationPropertyName> stream() {
		return StreamSupport.stream(getSource().spliterator(), false)
				.flatMap(this::addAliases);
	}

	private Stream<ConfigurationPropertyName> addAliases(ConfigurationPropertyName name) {
		Stream<ConfigurationPropertyName> names = Stream.of(name);
		List<ConfigurationPropertyName> aliases = getAliases().getAliases(name);
		if (CollectionUtils.isEmpty(aliases)) {
			return names;
		}
		return Stream.concat(names, aliases.stream());
	}

	@Override
	protected IterableConfigurationPropertySource getSource() {
		return (IterableConfigurationPropertySource) super.getSource();
	}

}
