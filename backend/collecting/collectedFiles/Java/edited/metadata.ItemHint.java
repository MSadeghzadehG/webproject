

package org.springframework.boot.configurationprocessor.metadata;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Map;


public class ItemHint implements Comparable<ItemHint> {

	private final String name;

	private final List<ValueHint> values;

	private final List<ValueProvider> providers;

	public ItemHint(String name, List<ValueHint> values, List<ValueProvider> providers) {
		this.name = toCanonicalName(name);
		this.values = (values != null ? new ArrayList<>(values) : new ArrayList<>());
		this.providers = (providers != null ? new ArrayList<>(providers)
				: new ArrayList<>());
	}

	private String toCanonicalName(String name) {
		int dot = name.lastIndexOf('.');
		if (dot != -1) {
			String prefix = name.substring(0, dot);
			String originalName = name.substring(dot);
			return prefix + ConfigurationMetadata.toDashedCase(originalName);
		}
		return ConfigurationMetadata.toDashedCase(name);
	}

	public String getName() {
		return this.name;
	}

	public List<ValueHint> getValues() {
		return Collections.unmodifiableList(this.values);
	}

	public List<ValueProvider> getProviders() {
		return Collections.unmodifiableList(this.providers);
	}

	@Override
	public int compareTo(ItemHint other) {
		return getName().compareTo(other.getName());
	}

	public static ItemHint newHint(String name, ValueHint... values) {
		return new ItemHint(name, Arrays.asList(values), Collections.emptyList());
	}

	@Override
	public String toString() {
		return "ItemHint{" + "name='" + this.name + "', values=" + this.values
				+ ", providers=" + this.providers + '}';
	}

	
	public static class ValueHint {

		private final Object value;

		private final String description;

		public ValueHint(Object value, String description) {
			this.value = value;
			this.description = description;
		}

		public Object getValue() {
			return this.value;
		}

		public String getDescription() {
			return this.description;
		}

		@Override
		public String toString() {
			return "ValueHint{" + "value=" + this.value + ", description='"
					+ this.description + '\'' + '}';
		}

	}

	
	public static class ValueProvider {

		private final String name;

		private final Map<String, Object> parameters;

		public ValueProvider(String name, Map<String, Object> parameters) {
			this.name = name;
			this.parameters = parameters;
		}

		public String getName() {
			return this.name;
		}

		public Map<String, Object> getParameters() {
			return this.parameters;
		}

		@Override
		public String toString() {
			return "ValueProvider{" + "name='" + this.name + "', parameters="
					+ this.parameters + '}';
		}

	}

}
