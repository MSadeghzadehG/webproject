

package org.springframework.boot.gradle.tasks.buildinfo;

import java.io.File;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;
import java.util.Map.Entry;

import org.gradle.api.Action;
import org.gradle.api.Project;
import org.gradle.api.Task;
import org.gradle.api.internal.ConventionTask;
import org.gradle.api.tasks.Input;
import org.gradle.api.tasks.OutputDirectory;
import org.gradle.api.tasks.TaskAction;
import org.gradle.api.tasks.TaskExecutionException;

import org.springframework.boot.loader.tools.BuildPropertiesWriter;
import org.springframework.boot.loader.tools.BuildPropertiesWriter.ProjectDetails;


public class BuildInfo extends ConventionTask {

	private final BuildInfoProperties properties = new BuildInfoProperties(getProject());

	private File destinationDir;

	
	@TaskAction
	public void generateBuildProperties() {
		try {
			new BuildPropertiesWriter(
					new File(getDestinationDir(), "build-info.properties"))
							.writeBuildProperties(new ProjectDetails(
									this.properties.getGroup(),
									this.properties.getArtifact() == null
											? "unspecified"
											: this.properties.getArtifact(),
									this.properties.getVersion(),
									this.properties.getName(), this.properties.getTime(),
									coerceToStringValues(
											this.properties.getAdditional())));
		}
		catch (IOException ex) {
			throw new TaskExecutionException(this, ex);
		}
	}

	
	@OutputDirectory
	public File getDestinationDir() {
		return this.destinationDir != null ? this.destinationDir
				: getProject().getBuildDir();
	}

	
	public void setDestinationDir(File destinationDir) {
		this.destinationDir = destinationDir;
	}

	
	@Input
	public BuildInfoProperties getProperties() {
		return this.properties;
	}

	
	public void properties(Action<BuildInfoProperties> action) {
		action.execute(this.properties);
	}

	private Map<String, String> coerceToStringValues(Map<String, Object> input) {
		Map<String, String> output = new HashMap<>();
		for (Entry<String, Object> entry : input.entrySet()) {
			output.put(entry.getKey(), entry.getValue().toString());
		}
		return output;
	}

}
