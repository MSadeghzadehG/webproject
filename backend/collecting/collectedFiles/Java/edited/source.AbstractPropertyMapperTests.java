

package org.springframework.boot.context.properties.source;

import java.util.Arrays;
import java.util.Iterator;


public abstract class AbstractPropertyMapperTests {

	protected abstract PropertyMapper getMapper();

	protected final Iterator<String> namesFromString(String name) {
		return namesFromString(name, "value");
	}

	protected final Iterator<String> namesFromString(String name, Object value) {
		return Arrays.stream(getMapper().map(name))
				.map((mapping) -> mapping.getConfigurationPropertyName().toString())
				.iterator();
	}

	protected final Iterator<String> namesFromConfiguration(String name) {
		return namesFromConfiguration(name, "value");
	}

	protected final Iterator<String> namesFromConfiguration(String name, String value) {
		return Arrays.stream(getMapper().map(ConfigurationPropertyName.of(name)))
				.map(PropertyMapping::getPropertySourceName).iterator();
	}

}
