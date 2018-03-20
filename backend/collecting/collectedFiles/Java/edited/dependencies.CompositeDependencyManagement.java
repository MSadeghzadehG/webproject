

package org.springframework.boot.cli.compiler.dependencies;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;


public class CompositeDependencyManagement implements DependencyManagement {

	private final List<DependencyManagement> delegates;

	private final List<Dependency> dependencies = new ArrayList<>();

	public CompositeDependencyManagement(DependencyManagement... delegates) {
		this.delegates = Arrays.asList(delegates);
		for (DependencyManagement delegate : delegates) {
			this.dependencies.addAll(delegate.getDependencies());
		}
	}

	@Override
	public List<Dependency> getDependencies() {
		return this.dependencies;
	}

	@Override
	public String getSpringBootVersion() {
		for (DependencyManagement delegate : this.delegates) {
			String version = delegate.getSpringBootVersion();
			if (version != null) {
				return version;
			}
		}
		return null;
	}

	@Override
	public Dependency find(String artifactId) {
		for (DependencyManagement delegate : this.delegates) {
			Dependency found = delegate.find(artifactId);
			if (found != null) {
				return found;
			}
		}
		return null;
	}

}
