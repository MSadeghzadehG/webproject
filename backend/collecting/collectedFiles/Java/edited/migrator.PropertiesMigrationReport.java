

package org.springframework.boot.context.properties.migrator;

import java.util.ArrayList;
import java.util.Collections;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.function.Function;
import java.util.stream.Collectors;

import org.springframework.boot.configurationmetadata.ConfigurationMetadataProperty;
import org.springframework.boot.configurationmetadata.Deprecation;
import org.springframework.util.StringUtils;


class PropertiesMigrationReport {

	private final Map<String, LegacyProperties> content = new LinkedHashMap<>();

	
	public String getWarningReport() {
		Map<String, List<PropertyMigration>> content = getContent(
				LegacyProperties::getRenamed);
		if (content.isEmpty()) {
			return null;
		}
		StringBuilder report = new StringBuilder();
		report.append(String.format("%nThe use of configuration keys that have been "
				+ "renamed was found in the environment:%n%n"));
		append(report, content, (metadata) -> "Replacement: "
				+ metadata.getDeprecation().getReplacement());
		report.append(String.format("%n"));
		report.append("Each configuration key has been temporarily mapped to its "
				+ "replacement for your convenience. To silence this warning, please "
				+ "update your configuration to use the new keys.");
		report.append(String.format("%n"));
		return report.toString();
	}

	
	public String getErrorReport() {
		Map<String, List<PropertyMigration>> content = getContent(
				LegacyProperties::getUnsupported);
		if (content.isEmpty()) {
			return null;
		}
		StringBuilder report = new StringBuilder();
		report.append(String.format("%nThe use of configuration keys that are no longer "
				+ "supported was found in the environment:%n%n"));
		append(report, content, this::determineReason);
		report.append(String.format("%n"));
		report.append("Please refer to the migration guide or reference guide for "
				+ "potential alternatives.");
		report.append(String.format("%n"));
		return report.toString();
	}

	private String determineReason(ConfigurationMetadataProperty metadata) {
		Deprecation deprecation = metadata.getDeprecation();
		if (StringUtils.hasText(deprecation.getShortReason())) {
			return "Reason: " + deprecation.getShortReason();
		}
		if (StringUtils.hasText(deprecation.getReplacement())) {
			return String.format(
					"Reason: Replacement key '%s' uses an incompatible " + "target type",
					deprecation.getReplacement());
		}
		return "Reason: none";
	}

	private Map<String, List<PropertyMigration>> getContent(
			Function<LegacyProperties, List<PropertyMigration>> extractor) {
		return this.content.entrySet().stream()
				.filter((entry) -> !extractor.apply(entry.getValue()).isEmpty())
				.collect(Collectors.toMap(Map.Entry::getKey,
						(entry) -> new ArrayList<>(extractor.apply(entry.getValue()))));
	}

	private void append(StringBuilder report,
			Map<String, List<PropertyMigration>> content,
			Function<ConfigurationMetadataProperty, String> deprecationMessage) {
		content.forEach((name, properties) -> {
			report.append(String.format("Property source '%s':%n", name));
			properties.sort(PropertyMigration.COMPARATOR);
			properties.forEach((property) -> {
				ConfigurationMetadataProperty metadata = property.getMetadata();
				report.append(String.format("\tKey: %s%n", metadata.getId()));
				if (property.getLineNumber() != null) {
					report.append(
							String.format("\t\tLine: %d%n", property.getLineNumber()));
				}
				report.append(
						String.format("\t\t%s%n", deprecationMessage.apply(metadata)));
			});
			report.append(String.format("%n"));
		});
	}

	
	void add(String name, List<PropertyMigration> renamed,
			List<PropertyMigration> unsupported) {
		this.content.put(name, new LegacyProperties(renamed, unsupported));
	}

	private static class LegacyProperties {

		private final List<PropertyMigration> renamed;

		private final List<PropertyMigration> unsupported;

		LegacyProperties(List<PropertyMigration> renamed,
				List<PropertyMigration> unsupported) {
			this.renamed = asNewList(renamed);
			this.unsupported = asNewList(unsupported);
		}

		private <T> List<T> asNewList(List<T> source) {
			return (source == null ? Collections.emptyList() : new ArrayList<>(source));
		}

		public List<PropertyMigration> getRenamed() {
			return this.renamed;
		}

		public List<PropertyMigration> getUnsupported() {
			return this.unsupported;
		}

	}

}
