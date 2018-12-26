

package org.springframework.boot.context.properties.source;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Map;

import org.springframework.util.Assert;
import org.springframework.util.LinkedMultiValueMap;
import org.springframework.util.MultiValueMap;


public final class ConfigurationPropertyNameAliases {

	private final MultiValueMap<ConfigurationPropertyName, ConfigurationPropertyName> aliases = new LinkedMultiValueMap<>();

	public ConfigurationPropertyNameAliases() {
	}

	public ConfigurationPropertyNameAliases(String name, String... aliases) {
		addAliases(name, aliases);
	}

	public ConfigurationPropertyNameAliases(ConfigurationPropertyName name,
			ConfigurationPropertyName... aliases) {
		addAliases(name, aliases);
	}

	public void addAliases(String name, String... aliases) {
		Assert.notNull(name, "Name must not be null");
		Assert.notNull(aliases, "Aliases must not be null");
		addAliases(ConfigurationPropertyName.of(name),
				Arrays.stream(aliases).map(ConfigurationPropertyName::of)
						.toArray(ConfigurationPropertyName[]::new));
	}

	public void addAliases(ConfigurationPropertyName name,
			ConfigurationPropertyName... aliases) {
		Assert.notNull(name, "Name must not be null");
		Assert.notNull(aliases, "Aliases must not be null");
		this.aliases.addAll(name, Arrays.asList(aliases));
	}

	public List<ConfigurationPropertyName> getAliases(ConfigurationPropertyName name) {
		return this.aliases.getOrDefault(name, Collections.emptyList());
	}

	public ConfigurationPropertyName getNameForAlias(ConfigurationPropertyName alias) {
		return this.aliases.entrySet().stream()
				.filter((e) -> e.getValue().contains(alias)).map(Map.Entry::getKey)
				.findFirst().orElse(null);
	}

}
