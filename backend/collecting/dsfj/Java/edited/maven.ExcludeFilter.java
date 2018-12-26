

package org.springframework.boot.maven;

import java.util.Arrays;
import java.util.List;

import org.apache.maven.artifact.Artifact;


public class ExcludeFilter extends DependencyFilter {

	public ExcludeFilter(Exclude... excludes) {
		this(Arrays.asList(excludes));
	}

	public ExcludeFilter(List<Exclude> excludes) {
		super(excludes);
	}

	@Override
	protected boolean filter(Artifact artifact) {
		for (FilterableDependency dependency : getFilters()) {
			if (equals(artifact, dependency)) {
				return true;
			}
		}
		return false;
	}

}
