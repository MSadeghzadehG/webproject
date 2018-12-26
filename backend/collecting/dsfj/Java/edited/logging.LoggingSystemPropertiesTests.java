

package org.springframework.boot.logging;

import java.util.Collections;
import java.util.HashSet;
import java.util.Set;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import org.springframework.core.env.Environment;
import org.springframework.core.env.MapPropertySource;
import org.springframework.core.env.StandardEnvironment;
import org.springframework.mock.env.MockEnvironment;

import static org.assertj.core.api.Assertions.assertThat;


public class LoggingSystemPropertiesTests {

	private Set<Object> systemPropertyNames;

	@Before
	public void captureSystemPropertyNames() {
		this.systemPropertyNames = new HashSet<>(System.getProperties().keySet());
	}

	@After
	public void restoreSystemProperties() {
		System.getProperties().keySet().retainAll(this.systemPropertyNames);
	}

	@Test
	public void pidIsSet() {
		new LoggingSystemProperties(new MockEnvironment()).apply(null);
		assertThat(System.getProperty(LoggingSystemProperties.PID_KEY)).isNotNull();
	}

	@Test
	public void consoleLogPatternIsSet() {
		new LoggingSystemProperties(new MockEnvironment()
				.withProperty("logging.pattern.console", "console pattern")).apply(null);
		assertThat(System.getProperty(LoggingSystemProperties.CONSOLE_LOG_PATTERN))
				.isEqualTo("console pattern");
	}

	@Test
	public void fileLogPatternIsSet() {
		new LoggingSystemProperties(new MockEnvironment()
				.withProperty("logging.pattern.file", "file pattern")).apply(null);
		assertThat(System.getProperty(LoggingSystemProperties.FILE_LOG_PATTERN))
				.isEqualTo("file pattern");
	}

	@Test
	public void consoleLogPatternCanReferencePid() {
		new LoggingSystemProperties(
				environment("logging.pattern.console", "${PID:unknown}")).apply(null);
		assertThat(System.getProperty(LoggingSystemProperties.CONSOLE_LOG_PATTERN))
				.matches("[0-9]+");
	}

	@Test
	public void fileLogPatternCanReferencePid() {
		new LoggingSystemProperties(environment("logging.pattern.file", "${PID:unknown}"))
				.apply(null);
		assertThat(System.getProperty(LoggingSystemProperties.FILE_LOG_PATTERN))
				.matches("[0-9]+");
	}

	private Environment environment(String key, Object value) {
		StandardEnvironment environment = new StandardEnvironment();
		environment.getPropertySources().addLast(
				new MapPropertySource("test", Collections.singletonMap(key, value)));
		return environment;
	}

}
