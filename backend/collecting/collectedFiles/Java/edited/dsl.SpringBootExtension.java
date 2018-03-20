

package org.springframework.boot.gradle.dsl;

import java.io.File;

import org.gradle.api.Action;
import org.gradle.api.Project;
import org.gradle.api.plugins.BasePlugin;
import org.gradle.api.plugins.JavaPlugin;
import org.gradle.api.plugins.JavaPluginConvention;
import org.gradle.api.tasks.SourceSet;
import org.gradle.jvm.tasks.Jar;

import org.springframework.boot.gradle.tasks.buildinfo.BuildInfo;
import org.springframework.boot.gradle.tasks.buildinfo.BuildInfoProperties;


public class SpringBootExtension {

	private final Project project;

	private String mainClassName;

	
	public SpringBootExtension(Project project) {
		this.project = project;
	}

	
	public String getMainClassName() {
		return this.mainClassName;
	}

	
	public void setMainClassName(String mainClassName) {
		this.mainClassName = mainClassName;
	}

	
	public void buildInfo() {
		this.buildInfo(null);
	}

	
	public void buildInfo(Action<BuildInfo> configurer) {
		BuildInfo bootBuildInfo = this.project.getTasks().create("bootBuildInfo",
				BuildInfo.class);
		bootBuildInfo.setGroup(BasePlugin.BUILD_GROUP);
		bootBuildInfo.setDescription("Generates a META-INF/build-info.properties file.");
		this.project.getPlugins().withType(JavaPlugin.class, (plugin) -> {
			this.project.getTasks().getByName(JavaPlugin.CLASSES_TASK_NAME)
					.dependsOn(bootBuildInfo);
			this.project.afterEvaluate((evaluated) -> {
				BuildInfoProperties properties = bootBuildInfo.getProperties();
				if (properties.getArtifact() == null) {
					properties.setArtifact(determineArtifactBaseName());
				}
			});
			bootBuildInfo.getConventionMapping().map("destinationDir",
					() -> new File(determineMainSourceSetResourcesOutputDir(),
							"META-INF"));
		});
		if (configurer != null) {
			configurer.execute(bootBuildInfo);
		}
	}

	private File determineMainSourceSetResourcesOutputDir() {
		return this.project.getConvention().getPlugin(JavaPluginConvention.class)
				.getSourceSets().getByName(SourceSet.MAIN_SOURCE_SET_NAME).getOutput()
				.getResourcesDir();
	}

	private String determineArtifactBaseName() {
		Jar artifactTask = findArtifactTask();
		return (artifactTask == null ? null : artifactTask.getBaseName());
	}

	private Jar findArtifactTask() {
		Jar artifactTask = (Jar) this.project.getTasks().findByName("bootWar");
		if (artifactTask != null) {
			return artifactTask;
		}
		return (Jar) this.project.getTasks().findByName("bootJar");
	}

}
