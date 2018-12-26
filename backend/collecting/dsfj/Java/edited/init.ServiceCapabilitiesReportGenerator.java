

package org.springframework.boot.cli.command.init;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.SortedSet;
import java.util.TreeSet;


class ServiceCapabilitiesReportGenerator {

	private static final String NEW_LINE = System.lineSeparator();

	private final InitializrService initializrService;

	
	ServiceCapabilitiesReportGenerator(InitializrService initializrService) {
		this.initializrService = initializrService;
	}

	
	public String generate(String url) throws IOException {
		Object content = this.initializrService.loadServiceCapabilities(url);
		if (content instanceof InitializrServiceMetadata) {
			return generateHelp(url, (InitializrServiceMetadata) content);
		}
		return content.toString();
	}

	private String generateHelp(String url, InitializrServiceMetadata metadata) {
		String header = "Capabilities of " + url;
		StringBuilder report = new StringBuilder();
		report.append(repeat("=", header.length()) + NEW_LINE);
		report.append(header + NEW_LINE);
		report.append(repeat("=", header.length()) + NEW_LINE);
		report.append(NEW_LINE);
		reportAvailableDependencies(metadata, report);
		report.append(NEW_LINE);
		reportAvailableProjectTypes(metadata, report);
		report.append(NEW_LINE);
		reportDefaults(report, metadata);
		return report.toString();
	}

	private void reportAvailableDependencies(InitializrServiceMetadata metadata,
			StringBuilder report) {
		report.append("Available dependencies:" + NEW_LINE);
		report.append("-----------------------" + NEW_LINE);
		List<Dependency> dependencies = getSortedDependencies(metadata);
		for (Dependency dependency : dependencies) {
			report.append(dependency.getId() + " - " + dependency.getName());
			if (dependency.getDescription() != null) {
				report.append(": " + dependency.getDescription());
			}
			report.append(NEW_LINE);
		}
	}

	private List<Dependency> getSortedDependencies(InitializrServiceMetadata metadata) {
		List<Dependency> dependencies = new ArrayList<>(metadata.getDependencies());
		dependencies.sort(Comparator.comparing(Dependency::getId));
		return dependencies;
	}

	private void reportAvailableProjectTypes(InitializrServiceMetadata metadata,
			StringBuilder report) {
		report.append("Available project types:" + NEW_LINE);
		report.append("------------------------" + NEW_LINE);
		SortedSet<Entry<String, ProjectType>> entries = new TreeSet<>(
				Comparator.comparing(Entry::getKey));
		entries.addAll(metadata.getProjectTypes().entrySet());
		for (Entry<String, ProjectType> entry : entries) {
			ProjectType type = entry.getValue();
			report.append(entry.getKey() + " -  " + type.getName());
			if (!type.getTags().isEmpty()) {
				reportTags(report, type);
			}
			if (type.isDefaultType()) {
				report.append(" (default)");
			}
			report.append(NEW_LINE);
		}
	}

	private void reportTags(StringBuilder report, ProjectType type) {
		Map<String, String> tags = type.getTags();
		Iterator<Map.Entry<String, String>> iterator = tags.entrySet().iterator();
		report.append(" [");
		while (iterator.hasNext()) {
			Map.Entry<String, String> entry = iterator.next();
			report.append(entry.getKey() + ":" + entry.getValue());
			if (iterator.hasNext()) {
				report.append(", ");
			}
		}
		report.append("]");
	}

	private void reportDefaults(StringBuilder report,
			InitializrServiceMetadata metadata) {
		report.append("Defaults:" + NEW_LINE);
		report.append("---------" + NEW_LINE);
		List<String> defaultsKeys = new ArrayList<>(metadata.getDefaults().keySet());
		Collections.sort(defaultsKeys);
		for (String defaultsKey : defaultsKeys) {
			String defaultsValue = metadata.getDefaults().get(defaultsKey);
			report.append(defaultsKey + ": " + defaultsValue + NEW_LINE);
		}
	}

	private static String repeat(String s, int count) {
		StringBuilder sb = new StringBuilder();
		for (int i = 0; i < count; i++) {
			sb.append(s);
		}
		return sb.toString();
	}

}
