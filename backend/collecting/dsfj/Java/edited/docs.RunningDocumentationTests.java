

package org.springframework.boot.gradle.docs;

import java.io.File;
import java.io.IOException;

import org.junit.Rule;
import org.junit.Test;

import org.springframework.boot.gradle.testkit.GradleBuild;

import static org.assertj.core.api.Assertions.assertThat;


public class RunningDocumentationTests {

	@Rule
	public GradleBuild gradleBuild = new GradleBuild();

	@Test
	public void bootRunMain() throws IOException {
		assertThat(this.gradleBuild.script("src/main/gradle/running/boot-run-main.gradle")
				.build("configuredMainClass").getOutput())
						.contains("com.example.ExampleApplication");
	}

	@Test
	public void applicationPluginMainClassName() throws IOException {
		assertThat(this.gradleBuild.script(
				"src/main/gradle/running/application-plugin-main-class-name.gradle")
				.build("configuredMainClass").getOutput())
						.contains("com.example.ExampleApplication");
	}

	@Test
	public void springBootDslMainClassName() throws IOException {
		assertThat(this.gradleBuild
				.script("src/main/gradle/running/spring-boot-dsl-main-class-name.gradle")
				.build("configuredMainClass").getOutput())
						.contains("com.example.ExampleApplication");
	}

	@Test
	public void bootRunSourceResources() throws IOException {
		assertThat(this.gradleBuild
				.script("src/main/gradle/running/boot-run-source-resources.gradle")
				.build("configuredClasspath").getOutput())
						.contains(new File("src/main/resources").getPath());
	}

}
