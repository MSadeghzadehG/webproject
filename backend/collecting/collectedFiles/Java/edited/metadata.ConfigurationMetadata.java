

package org.springframework.boot.configurationprocessor.metadata;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashSet;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Set;


public class ConfigurationMetadata {

	private static final Set<Character> SEPARATORS;

	static {
		List<Character> chars = Arrays.asList('-', '_');
		SEPARATORS = Collections.unmodifiableSet(new HashSet<>(chars));
	}

	private final Map<String, List<ItemMetadata>> items;

	private final Map<String, List<ItemHint>> hints;

	public ConfigurationMetadata() {
		this.items = new LinkedHashMap<>();
		this.hints = new LinkedHashMap<>();
	}

	public ConfigurationMetadata(ConfigurationMetadata metadata) {
		this.items = new LinkedHashMap<>(metadata.items);
		this.hints = new LinkedHashMap<>(metadata.hints);
	}

	
	public void add(ItemMetadata itemMetadata) {
		add(this.items, itemMetadata.getName(), itemMetadata);
	}

	
	public void add(ItemHint itemHint) {
		add(this.hints, itemHint.getName(), itemHint);
	}

	
	public void merge(ConfigurationMetadata metadata) {
		for (ItemMetadata additionalItem : metadata.getItems()) {
			mergeItemMetadata(additionalItem);
		}
		for (ItemHint itemHint : metadata.getHints()) {
			add(itemHint);
		}
	}

	
	public List<ItemMetadata> getItems() {
		return flattenValues(this.items);
	}

	
	public List<ItemHint> getHints() {
		return flattenValues(this.hints);
	}

	protected void mergeItemMetadata(ItemMetadata metadata) {
		ItemMetadata matching = findMatchingItemMetadata(metadata);
		if (matching != null) {
			if (metadata.getDescription() != null) {
				matching.setDescription(metadata.getDescription());
			}
			if (metadata.getDefaultValue() != null) {
				matching.setDefaultValue(metadata.getDefaultValue());
			}
			ItemDeprecation deprecation = metadata.getDeprecation();
			ItemDeprecation matchingDeprecation = matching.getDeprecation();
			if (deprecation != null) {
				if (matchingDeprecation == null) {
					matching.setDeprecation(deprecation);
				}
				else {
					if (deprecation.getReason() != null) {
						matchingDeprecation.setReason(deprecation.getReason());
					}
					if (deprecation.getReplacement() != null) {
						matchingDeprecation.setReplacement(deprecation.getReplacement());
					}
					if (deprecation.getLevel() != null) {
						matchingDeprecation.setLevel(deprecation.getLevel());
					}
				}
			}
		}
		else {
			add(this.items, metadata.getName(), metadata);
		}
	}

	private <K, V> void add(Map<K, List<V>> map, K key, V value) {
		List<V> values = map.get(key);
		if (values == null) {
			values = new ArrayList<>();
			map.put(key, values);
		}
		values.add(value);
	}

	private ItemMetadata findMatchingItemMetadata(ItemMetadata metadata) {
		List<ItemMetadata> candidates = this.items.get(metadata.getName());
		if (candidates == null || candidates.isEmpty()) {
			return null;
		}
		candidates.removeIf((itemMetadata) -> !itemMetadata.hasSameType(metadata));
		if (candidates.size() == 1) {
			return candidates.get(0);
		}
		for (ItemMetadata candidate : candidates) {
			if (nullSafeEquals(candidate.getSourceType(), metadata.getSourceType())) {
				return candidate;
			}
		}
		return null;
	}

	private boolean nullSafeEquals(Object o1, Object o2) {
		if (o1 == o2) {
			return true;
		}
		return o1 != null && o2 != null && o1.equals(o2);
	}

	public static String nestedPrefix(String prefix, String name) {
		String nestedPrefix = (prefix == null ? "" : prefix);
		String dashedName = toDashedCase(name);
		nestedPrefix += ("".equals(nestedPrefix) ? dashedName : "." + dashedName);
		return nestedPrefix;
	}

	static String toDashedCase(String name) {
		StringBuilder dashed = new StringBuilder();
		Character previous = null;
		for (char current : name.toCharArray()) {
			if (SEPARATORS.contains(current)) {
				dashed.append("-");
			}
			else if (Character.isUpperCase(current) && previous != null
					&& !SEPARATORS.contains(previous)) {
				dashed.append("-").append(current);
			}
			else {
				dashed.append(current);
			}
			previous = current;

		}
		return dashed.toString().toLowerCase(Locale.ENGLISH);
	}

	private static <T extends Comparable<T>> List<T> flattenValues(Map<?, List<T>> map) {
		List<T> content = new ArrayList<>();
		for (List<T> values : map.values()) {
			content.addAll(values);
		}
		Collections.sort(content);
		return content;
	}

	@Override
	public String toString() {
		StringBuilder result = new StringBuilder();
		result.append(String.format("items: %n"));
		this.items.values().forEach((itemMetadata) -> result.append("\t")
				.append(String.format("%s%n", itemMetadata)));
		return result.toString();
	}

}
