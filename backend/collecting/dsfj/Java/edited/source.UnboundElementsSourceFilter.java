

package org.springframework.boot.context.properties.source;

import java.util.Arrays;
import java.util.Collections;
import java.util.HashSet;
import java.util.Set;
import java.util.function.Function;

import org.springframework.core.env.PropertySource;
import org.springframework.core.env.StandardEnvironment;


public class UnboundElementsSourceFilter
		implements Function<ConfigurationPropertySource, Boolean> {

	private static final Set<String> BENIGN_PROPERTY_SOURCE_NAMES = Collections
			.unmodifiableSet(new HashSet<>(Arrays.asList(
					StandardEnvironment.SYSTEM_ENVIRONMENT_PROPERTY_SOURCE_NAME,
					StandardEnvironment.SYSTEM_PROPERTIES_PROPERTY_SOURCE_NAME)));

	@Override
	public Boolean apply(ConfigurationPropertySource configurationPropertySource) {
		Object underlyingSource = configurationPropertySource.getUnderlyingSource();
		if (underlyingSource instanceof PropertySource) {
			String name = ((PropertySource<?>) underlyingSource).getName();
			return !BENIGN_PROPERTY_SOURCE_NAMES.contains(name);

		}
		return true;
	}

}
