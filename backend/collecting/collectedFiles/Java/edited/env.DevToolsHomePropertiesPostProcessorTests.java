

package org.springframework.boot.devtools.env;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.util.Properties;

import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;

import org.springframework.core.env.ConfigurableEnvironment;
import org.springframework.mock.env.MockEnvironment;

import static org.assertj.core.api.Assertions.assertThat;


public class DevToolsHomePropertiesPostProcessorTests {

	@Rule
	public TemporaryFolder temp = new TemporaryFolder();

	private File home;

	@Before
	public void setup() throws IOException {
		this.home = this.temp.newFolder();
	}

	@Test
	public void loadsHomeProperties() throws Exception {
		Properties properties = new Properties();
		properties.put("abc", "def");
		OutputStream out = new FileOutputStream(
				new File(this.home, ".spring-boot-devtools.properties"));
		properties.store(out, null);
		out.close();
		ConfigurableEnvironment environment = new MockEnvironment();
		MockDevToolHomePropertiesPostProcessor postProcessor = new MockDevToolHomePropertiesPostProcessor();
		postProcessor.postProcessEnvironment(environment, null);
		assertThat(environment.getProperty("abc")).isEqualTo("def");
	}

	@Test
	public void ignoresMissingHomeProperties() {
		ConfigurableEnvironment environment = new MockEnvironment();
		MockDevToolHomePropertiesPostProcessor postProcessor = new MockDevToolHomePropertiesPostProcessor();
		postProcessor.postProcessEnvironment(environment, null);
		assertThat(environment.getProperty("abc")).isNull();
	}

	private class MockDevToolHomePropertiesPostProcessor
			extends DevToolsHomePropertiesPostProcessor {

		@Override
		protected File getHomeFolder() {
			return DevToolsHomePropertiesPostProcessorTests.this.home;
		}

	}

}
