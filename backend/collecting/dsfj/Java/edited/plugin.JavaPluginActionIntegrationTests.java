

package org.springframework.boot.gradle.plugin;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.jar.JarOutputStream;

import org.gradle.testkit.runner.BuildResult;
import org.gradle.testkit.runner.TaskOutcome;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;

import org.springframework.boot.gradle.junit.GradleCompatibilitySuite;
import org.springframework.boot.gradle.testkit.GradleBuild;

import static org.assertj.core.api.Assertions.assertThat;


@RunWith(GradleCompatibilitySuite.class)
public class JavaPluginActionIntegrationTests {

	@Rule
	public GradleBuild gradleBuild;

	@Test
	public void noBootJarTaskWithoutJavaPluginApplied() {
		assertThat(this.gradleBuild.build("taskExists", "-PtaskName=bootJar").getOutput())
				.contains("bootJar exists = false");
	}

	@Test
	public void applyingJavaPluginCreatesBootJarTask() {
		assertThat(this.gradleBuild
				.build("taskExists", "-PtaskName=bootJar", "-PapplyJavaPlugin")
				.getOutput()).contains("bootJar exists = true");
	}

	@Test
	public void noBootRunTaskWithoutJavaPluginApplied() {
		assertThat(this.gradleBuild.build("taskExists", "-PtaskName=bootRun").getOutput())
				.contains("bootRun exists = false");
	}

	@Test
	public void applyingJavaPluginCreatesBootRunTask() {
		assertThat(this.gradleBuild
				.build("taskExists", "-PtaskName=bootRun", "-PapplyJavaPlugin")
				.getOutput()).contains("bootRun exists = true");
	}

	@Test
	public void javaCompileTasksUseUtf8Encoding() {
		assertThat(this.gradleBuild.build("javaCompileEncoding", "-PapplyJavaPlugin")
				.getOutput()).contains("compileJava = UTF-8")
						.contains("compileTestJava = UTF-8");
	}

	@Test
	public void javaCompileTasksUseParametersCompilerFlagByDefault() {
		assertThat(this.gradleBuild.build("javaCompileTasksCompilerArgs").getOutput())
				.contains("compileJava compiler args: [-parameters]")
				.contains("compileTestJava compiler args: [-parameters]");
	}

	@Test
	public void javaCompileTasksUseParametersAndAdditionalCompilerFlags() {
		assertThat(this.gradleBuild.build("javaCompileTasksCompilerArgs").getOutput())
				.contains("compileJava compiler args: [-parameters, -Xlint:all]")
				.contains("compileTestJava compiler args: [-parameters, -Xlint:all]");
	}

	@Test
	public void javaCompileTasksCanOverrideDefaultParametersCompilerFlag() {
		assertThat(this.gradleBuild.build("javaCompileTasksCompilerArgs").getOutput())
				.contains("compileJava compiler args: [-Xlint:all]")
				.contains("compileTestJava compiler args: [-Xlint:all]");
	}

	@Test
	public void assembleRunsBootJarAndJarIsSkipped() {
		BuildResult result = this.gradleBuild.build("assemble");
		assertThat(result.task(":bootJar").getOutcome()).isEqualTo(TaskOutcome.SUCCESS);
		assertThat(result.task(":jar").getOutcome()).isEqualTo(TaskOutcome.SKIPPED);
	}

	@Test
	public void jarAndBootJarCanBothBeBuilt() {
		BuildResult result = this.gradleBuild.build("assemble");
		assertThat(result.task(":bootJar").getOutcome()).isEqualTo(TaskOutcome.SUCCESS);
		assertThat(result.task(":jar").getOutcome()).isEqualTo(TaskOutcome.SUCCESS);
		File buildLibs = new File(this.gradleBuild.getProjectDir(), "build/libs");
		assertThat(buildLibs.listFiles()).containsExactlyInAnyOrder(
				new File(buildLibs, this.gradleBuild.getProjectDir().getName() + ".jar"),
				new File(buildLibs,
						this.gradleBuild.getProjectDir().getName() + "-boot.jar"));
	}

	@Test
	public void additionalMetadataLocationsConfiguredWhenProcessorIsPresent()
			throws IOException {
		createMinimalMainSource();
		File libs = new File(this.gradleBuild.getProjectDir(), "libs");
		libs.mkdirs();
		new JarOutputStream(new FileOutputStream(
				new File(libs, "spring-boot-configuration-processor-1.2.3.jar"))).close();
		BuildResult result = this.gradleBuild.build("compileJava");
		assertThat(result.task(":compileJava").getOutcome())
				.isEqualTo(TaskOutcome.SUCCESS);
		assertThat(result.getOutput()).contains(
				"compileJava compiler args: [-parameters, -Aorg.springframework.boot."
						+ "configurationprocessor.additionalMetadataLocations="
						+ new File(this.gradleBuild.getProjectDir(), "src/main/resources")
								.getCanonicalPath());
	}

	@Test
	public void additionalMetadataLocationsNotConfiguredWhenProcessorIsAbsent()
			throws IOException {
		createMinimalMainSource();
		BuildResult result = this.gradleBuild.build("compileJava");
		assertThat(result.task(":compileJava").getOutcome())
				.isEqualTo(TaskOutcome.SUCCESS);
		assertThat(result.getOutput())
				.contains("compileJava compiler args: [-parameters]");
	}

	private void createMinimalMainSource() throws IOException {
		File examplePackage = new File(this.gradleBuild.getProjectDir(),
				"src/main/java/com/example");
		examplePackage.mkdirs();
		new File(examplePackage, "Application.java").createNewFile();
	}

}
