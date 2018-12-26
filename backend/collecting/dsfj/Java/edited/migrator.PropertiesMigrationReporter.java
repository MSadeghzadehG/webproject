

package org.springframework.boot.context.properties.migrator;

import java.time.Duration;
import java.util.ArrayList;
import java.util.Collections;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.function.Predicate;
import java.util.stream.Collectors;

import org.springframework.boot.configurationmetadata.ConfigurationMetadataProperty;
import org.springframework.boot.configurationmetadata.ConfigurationMetadataRepository;
import org.springframework.boot.configurationmetadata.Deprecation;
import org.springframework.boot.context.properties.source.ConfigurationProperty;
import org.springframework.boot.context.properties.source.ConfigurationPropertyName;
import org.springframework.boot.context.properties.source.ConfigurationPropertySource;
import org.springframework.boot.context.properties.source.ConfigurationPropertySources;
import org.springframework.boot.env.OriginTrackedMapPropertySource;
import org.springframework.boot.origin.OriginTrackedValue;
import org.springframework.core.env.ConfigurableEnvironment;
import org.springframework.core.env.PropertySource;
import org.springframework.util.LinkedMultiValueMap;
import org.springframework.util.MultiValueMap;
import org.springframework.util.StringUtils;


class PropertiesMigrationReporter {

	private final Map<String, ConfigurationMetadataProperty> allProperties;

	private final ConfigurableEnvironment environment;

	PropertiesMigrationReporter(ConfigurationMetadataRepository metadataRepository,
			ConfigurableEnvironment environment) {
		this.allProperties = Collections
				.unmodifiableMap(metadataRepository.getAllProperties());
		this.environment = environment;
	}

	
	public PropertiesMigrationReport getReport() {
		PropertiesMigrationReport report = new PropertiesMigrationReport();
		Map<String, List<PropertyMigration>> properties = getMatchingProperties(
				deprecatedFilter());
		if (properties.isEmpty()) {
			return report;
		}
		properties.forEach((name, candidates) -> {
			PropertySource<?> propertySource = mapPropertiesWithReplacement(report, name,
					candidates);
			if (propertySource != null) {
				this.environment.getPropertySources().addBefore(name, propertySource);
			}
		});
		return report;
	}

	private PropertySource<?> mapPropertiesWithReplacement(
			PropertiesMigrationReport report, String name,
			List<PropertyMigration> properties) {
		List<PropertyMigration> renamed = new ArrayList<>();
		List<PropertyMigration> unsupported = new ArrayList<>();
		properties.forEach((property) -> (isRenamed(property) ? renamed : unsupported)
				.add(property));
		report.add(name, renamed, unsupported);
		if (renamed.isEmpty()) {
			return null;
		}
		String target = "migrate-" + name;
		Map<String, OriginTrackedValue> content = new LinkedHashMap<>();
		for (PropertyMigration candidate : renamed) {
			OriginTrackedValue value = OriginTrackedValue.of(
					candidate.getProperty().getValue(),
					candidate.getProperty().getOrigin());
			content.put(candidate.getMetadata().getDeprecation().getReplacement(), value);
		}
		return new OriginTrackedMapPropertySource(target, content);
	}

	private boolean isRenamed(PropertyMigration property) {
		ConfigurationMetadataProperty metadata = property.getMetadata();
		String replacementId = metadata.getDeprecation().getReplacement();
		if (StringUtils.hasText(replacementId)) {
			ConfigurationMetadataProperty replacement = this.allProperties
					.get(replacementId);
			if (replacement != null) {
				return isCompatibleType(metadata.getType(), replacement.getType());
			}
			return isCompatibleType(metadata.getType(),
					detectMapValueReplacementType(replacementId));
		}
		return false;
	}

	private boolean isCompatibleType(String currentType, String replacementType) {
		if (replacementType == null || currentType == null) {
			return false;
		}
		if (replacementType.equals(currentType)) {
			return true;
		}
		if (replacementType.equals(Duration.class.getName())
				&& (currentType.equals(Long.class.getName())
						|| currentType.equals(Integer.class.getName()))) {
			return true;
		}
		return false;
	}

	private String detectMapValueReplacementType(String fullId) {
		int lastDot = fullId.lastIndexOf('.');
		if (lastDot != -1) {
			ConfigurationMetadataProperty property = this.allProperties
					.get(fullId.substring(0, lastDot));
			String type = property.getType();
			if (type != null && type.startsWith(Map.class.getName())) {
				int lastComma = type.lastIndexOf(',');
				if (lastComma != -1) {
					return type.substring(lastComma + 1, type.length() - 1).trim();
				}
			}
		}
		return null;
	}

	private Map<String, List<PropertyMigration>> getMatchingProperties(
			Predicate<ConfigurationMetadataProperty> filter) {
		MultiValueMap<String, PropertyMigration> result = new LinkedMultiValueMap<>();
		List<ConfigurationMetadataProperty> candidates = this.allProperties.values()
				.stream().filter(filter).collect(Collectors.toList());
		getPropertySourcesAsMap().forEach((name, source) -> {
			candidates.forEach((metadata) -> {
				ConfigurationProperty configurationProperty = source
						.getConfigurationProperty(
								ConfigurationPropertyName.of(metadata.getId()));
				if (configurationProperty != null) {
					result.add(name,
							new PropertyMigration(metadata, configurationProperty));
				}
			});
		});
		return result;
	}

	private Predicate<ConfigurationMetadataProperty> deprecatedFilter() {
		return (property) -> property.getDeprecation() != null
				&& property.getDeprecation().getLevel() == Deprecation.Level.ERROR;
	}

	private Map<String, ConfigurationPropertySource> getPropertySourcesAsMap() {
		Map<String, ConfigurationPropertySource> map = new LinkedHashMap<>();
		ConfigurationPropertySources.get(this.environment);
		for (ConfigurationPropertySource source : ConfigurationPropertySources
				.get(this.environment)) {
			map.put(determinePropertySourceName(source), source);
		}
		return map;
	}

	private String determinePropertySourceName(ConfigurationPropertySource source) {
		if (source.getUnderlyingSource() instanceof PropertySource) {
			return ((PropertySource<?>) source.getUnderlyingSource()).getName();
		}
		return source.getUnderlyingSource().toString();
	}

}
