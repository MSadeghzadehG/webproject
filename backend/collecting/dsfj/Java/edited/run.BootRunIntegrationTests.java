

package org.springframework.boot.gradle.tasks.run;

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
public class BootRunIntegrationTests {

	@Rule
	public GradleBuild gradleBuild;

	@Test
	public void basicExecution() throws IOException {
		copyApplication();
		new File(this.gradleBuild.getProjectDir(), "src/main/resources").mkdirs();
		BuildResult result = this.gradleBuild.build("bootRun");
		assertThat(result.task(":bootRun").getOutcome()).isEqualTo(TaskOutcome.SUCCESS);
		assertThat(result.getOutput())
				.contains("1. " + canonicalPathOf("build/classes/java/main"));
		assertThat(result.getOutput())
				.contains("2. " + canonicalPathOf("build/resources/main"));
		assertThat(result.getOutput())
				.doesNotContain(canonicalPathOf("src/main/resources"));
	}

	@Test
	public void sourceResourcesCanBeUsed() throws IOException {
		copyApplication();
		BuildResult result = this.gradleBuild.build("bootRun");
		assertThat(result.task(":bootRun").getOutcome()).isEqualTo(TaskOutcome.SUCCESS);
		assertThat(result.getOutput())
				.contains("1. " + canonicalPathOf("src/main/resources"));
		assertThat(result.getOutput())
				.contains("2. " + canonicalPathOf("build/classes/java/main"));
		assertThat(result.getOutput())
				.doesNotContain(canonicalPathOf("build/resources/main"));
	}

	@Test
	public void springBootExtensionMainClassNameIsUsed() throws IOException {
		BuildResult result = this.gradleBuild.build("echoMainClassName");
		assertThat(result.task(":echoMainClassName").getOutcome())
				.isEqualTo(TaskOutcome.UP_TO_DATE);
		assertThat(result.getOutput())
				.contains("Main class name = com.example.CustomMainClass");
	}

	@Test
	public void applicationPluginMainClassNameIsUsed() throws IOException {
		BuildResult result = this.gradleBuild.build("echoMainClassName");
		assertThat(result.task(":echoMainClassName").getOutcome())
				.isEqualTo(TaskOutcome.UP_TO_DATE);
		assertThat(result.getOutput())
				.contains("Main class name = com.example.CustomMainClass");
	}

	@Test
	public void applicationPluginMainClassNameIsNotUsedWhenItIsNull() throws IOException {
		copyApplication();
		BuildResult result = this.gradleBuild.build("echoMainClassName");
		assertThat(result.task(":echoMainClassName").getOutcome())
				.isEqualTo(TaskOutcome.SUCCESS);
		assertThat(result.getOutput())
				.contains("Main class name = com.example.BootRunApplication");
	}

	@Test
	public void applicationPluginJvmArgumentsAreUsed() throws IOException {
		BuildResult result = this.gradleBuild.build("echoJvmArguments");
		assertThat(result.task(":echoJvmArguments").getOutcome())
				.isEqualTo(TaskOutcome.UP_TO_DATE);
		assertThat(result.getOutput())
				.contains("JVM arguments = [-Dcom.foo=bar, -Dcom.bar=baz]");
	}

	private void copyApplication() throws IOException {
		File output = new File(this.gradleBuild.getProjectDir(),
				"src/main/java/com/example");
		output.mkdirs();
		FileSystemUtils.copyRecursively(new File("src/test/java/com/example"), output);
	}

	private String canonicalPathOf(String path) throws IOException {
		return new File(this.gradleBuild.getProjectDir(), path).getCanonicalPath();
	}

}
