

package org.springframework.boot.gradle.plugin;

import java.util.HashSet;
import java.util.Set;
import java.util.stream.Collectors;

import io.spring.gradle.dependencymanagement.DependencyManagementPlugin;
import org.gradle.api.Project;
import org.gradle.api.artifacts.ModuleVersionSelector;
import org.gradle.api.artifacts.UnresolvedDependency;
import org.slf4j.LoggerFactory;


class UnresolvedDependenciesAnalyzer {

	private static final org.slf4j.Logger logger = LoggerFactory
			.getLogger(SpringBootPlugin.class);

	private Set<ModuleVersionSelector> dependenciesWithNoVersion = new HashSet<>();

	void analyze(Set<UnresolvedDependency> unresolvedDependencies) {
		this.dependenciesWithNoVersion = unresolvedDependencies.stream()
				.map((unresolvedDependency) -> unresolvedDependency.getSelector())
				.filter(this::hasNoVersion).collect(Collectors.toSet());
	}

	void buildFinished(Project project) {
		if (!this.dependenciesWithNoVersion.isEmpty()
				&& !project.getPlugins().hasPlugin(DependencyManagementPlugin.class)) {
			StringBuilder message = new StringBuilder();
			message.append("\nDuring the build, one or more dependencies that were "
					+ "declared without a version failed to resolve:\n");
			this.dependenciesWithNoVersion
					.forEach((dependency) -> message.append("    " + dependency + "\n"));
			message.append("\nDid you forget to apply the "
					+ "io.spring.dependency-management plugin to the " + project.getName()
					+ " project?\n");
			logger.warn(message.toString());
		}
	}

	private boolean hasNoVersion(ModuleVersionSelector selector) {
		String version = selector.getVersion();
		return version == null || version.trim().isEmpty();
	}

}
