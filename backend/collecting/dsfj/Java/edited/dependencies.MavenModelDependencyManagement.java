

package org.springframework.boot.cli.compiler.dependencies;

import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import org.apache.maven.model.Model;

import org.springframework.boot.cli.compiler.dependencies.Dependency.Exclusion;


public class MavenModelDependencyManagement implements DependencyManagement {

	private final List<Dependency> dependencies;

	private final Map<String, Dependency> byArtifactId = new LinkedHashMap<>();

	public MavenModelDependencyManagement(Model model) {
		this.dependencies = extractDependenciesFromModel(model);
		for (Dependency dependency : this.dependencies) {
			this.byArtifactId.put(dependency.getArtifactId(), dependency);
		}
	}

	private static List<Dependency> extractDependenciesFromModel(Model model) {
		List<Dependency> dependencies = new ArrayList<>();
		for (org.apache.maven.model.Dependency mavenDependency : model
				.getDependencyManagement().getDependencies()) {
			List<Exclusion> exclusions = new ArrayList<>();
			for (org.apache.maven.model.Exclusion mavenExclusion : mavenDependency
					.getExclusions()) {
				exclusions.add(new Exclusion(mavenExclusion.getGroupId(),
						mavenExclusion.getArtifactId()));
			}
			Dependency dependency = new Dependency(mavenDependency.getGroupId(),
					mavenDependency.getArtifactId(), mavenDependency.getVersion(),
					exclusions);
			dependencies.add(dependency);
		}
		return dependencies;
	}

	@Override
	public List<Dependency> getDependencies() {
		return this.dependencies;
	}

	@Override
	public String getSpringBootVersion() {
		return find("spring-boot").getVersion();
	}

	@Override
	public Dependency find(String artifactId) {
		return this.byArtifactId.get(artifactId);
	}

}
