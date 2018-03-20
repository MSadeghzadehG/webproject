

package org.springframework.boot.gradle.plugin;

import java.io.File;
import java.io.IOException;

import org.gradle.testkit.runner.BuildResult;
import org.gradle.testkit.runner.TaskOutcome;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;

import org.springframework.boot.gradle.junit.GradleCompatibilitySuite;
import org.springframework.boot.gradle.testkit.GradleBuild;
import org.springframework.util.FileSystemUtils;

import static org.assertj.core.api.Assertions.assertThat;


@RunWith(GradleCompatibilitySuite.class)
public class DependencyManagementPluginActionIntegrationTests {

	@Rule
	public GradleBuild gradleBuild;

	@Test
	public void noDependencyManagementIsAppliedByDefault() {
		assertThat(this.gradleBuild.build("doesNotHaveDependencyManagement")
				.task(":doesNotHaveDependencyManagement").getOutcome())
						.isEqualTo(TaskOutcome.SUCCESS);
	}

	@Test
	public void bomIsImportedWhenDependencyManagementPluginIsApplied() {
		assertThat(this.gradleBuild
				.build("hasDependencyManagement", "-PapplyDependencyManagementPlugin")
				.task(":hasDependencyManagement").getOutcome())
						.isEqualTo(TaskOutcome.SUCCESS);
	}

	@Test
	public void helpfulErrorWhenVersionlessDependencyFailsToResolve() throws IOException {
		File examplePackage = new File(this.gradleBuild.getProjectDir(),
				"src/main/java/com/example");
		examplePackage.mkdirs();
		FileSystemUtils.copyRecursively(new File("src/test/java/com/example"),
				examplePackage);
		BuildResult result = this.gradleBuild.buildAndFail("compileJava");
		assertThat(result.task(":compileJava").getOutcome())
				.isEqualTo(TaskOutcome.FAILED);
		String output = result.getOutput();
		assertThat(output).contains("During the build, one or more dependencies that "
				+ "were declared without a version failed to resolve:");
		assertThat(output).contains("org.springframework.boot:spring-boot-starter-web:");
		assertThat(output).contains("Did you forget to apply the "
				+ "io.spring.dependency-management plugin to the "
				+ this.gradleBuild.getProjectDir().getName() + " project?");
	}

}
