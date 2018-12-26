

package org.springframework.boot.maven;

import java.io.File;
import java.time.Instant;
import java.util.Map;

import org.apache.maven.plugin.AbstractMojo;
import org.apache.maven.plugin.MojoExecutionException;
import org.apache.maven.plugin.MojoFailureException;
import org.apache.maven.plugins.annotations.Component;
import org.apache.maven.plugins.annotations.LifecyclePhase;
import org.apache.maven.plugins.annotations.Mojo;
import org.apache.maven.plugins.annotations.Parameter;
import org.apache.maven.project.MavenProject;
import org.sonatype.plexus.build.incremental.BuildContext;

import org.springframework.boot.loader.tools.BuildPropertiesWriter;
import org.springframework.boot.loader.tools.BuildPropertiesWriter.NullAdditionalPropertyValueException;
import org.springframework.boot.loader.tools.BuildPropertiesWriter.ProjectDetails;


@Mojo(name = "build-info", defaultPhase = LifecyclePhase.GENERATE_RESOURCES, threadSafe = true)
public class BuildInfoMojo extends AbstractMojo {

	@Component
	private BuildContext buildContext;

	
	@Parameter(defaultValue = "${project}", readonly = true, required = true)
	private MavenProject project;

	
	@Parameter(defaultValue = "${project.build.outputDirectory}/META-INF/build-info.properties")
	private File outputFile;

	
	@Parameter
	private Map<String, String> additionalProperties;

	@Override
	public void execute() throws MojoExecutionException, MojoFailureException {
		try {
			new BuildPropertiesWriter(this.outputFile)
					.writeBuildProperties(new ProjectDetails(this.project.getGroupId(),
							this.project.getArtifactId(), this.project.getVersion(),
							this.project.getName(), Instant.now(),
							this.additionalProperties));
			this.buildContext.refresh(this.outputFile);
		}
		catch (NullAdditionalPropertyValueException ex) {
			throw new MojoFailureException(
					"Failed to generate build-info.properties. " + ex.getMessage(), ex);
		}
		catch (Exception ex) {
			throw new MojoExecutionException(ex.getMessage(), ex);
		}
	}

}
