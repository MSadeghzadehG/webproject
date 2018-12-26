

package org.springframework.boot.gradle.junit;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.gradle.api.Rule;
import org.junit.runner.Runner;
import org.junit.runners.BlockJUnit4ClassRunner;
import org.junit.runners.Suite;
import org.junit.runners.model.FrameworkMethod;
import org.junit.runners.model.InitializationError;

import org.springframework.boot.gradle.testkit.GradleBuild;


public final class GradleCompatibilitySuite extends Suite {

	private static final List<String> GRADLE_VERSIONS = Arrays.asList("default", "4.1",
			"4.2", "4.3", "4.4.1", "4.5.1", "4.6");

	public GradleCompatibilitySuite(Class<?> clazz) throws InitializationError {
		super(clazz, createRunners(clazz));
	}

	private static List<Runner> createRunners(Class<?> clazz) throws InitializationError {
		List<Runner> runners = new ArrayList<>();
		for (String version : GRADLE_VERSIONS) {
			runners.add(new GradleCompatibilityClassRunner(clazz, version));
		}
		return runners;
	}

	private static final class GradleCompatibilityClassRunner
			extends BlockJUnit4ClassRunner {

		private final String gradleVersion;

		private GradleCompatibilityClassRunner(Class<?> klass, String gradleVersion)
				throws InitializationError {
			super(klass);
			this.gradleVersion = gradleVersion;
		}

		@Override
		protected Object createTest() throws Exception {
			Object test = super.createTest();
			configureTest(test);
			return test;
		}

		private void configureTest(Object test) throws Exception {
			GradleBuild gradleBuild = new GradleBuild();
			if (!"default".equals(this.gradleVersion)) {
				gradleBuild = gradleBuild.gradleVersion(this.gradleVersion);
			}
			test.getClass().getField("gradleBuild").set(test, gradleBuild);
		}

		@Override
		protected String getName() {
			return "Gradle " + this.gradleVersion;
		}

		@Override
		protected String testName(FrameworkMethod method) {
			return method.getName() + " [" + getName() + "]";
		}

	}

}
