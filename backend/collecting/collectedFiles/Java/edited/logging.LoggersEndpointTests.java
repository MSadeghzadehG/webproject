

package org.springframework.boot.actuate.logging;

import java.util.Collections;
import java.util.EnumSet;
import java.util.Map;
import java.util.Set;

import org.junit.Test;

import org.springframework.boot.actuate.logging.LoggersEndpoint.LoggerLevels;
import org.springframework.boot.logging.LogLevel;
import org.springframework.boot.logging.LoggerConfiguration;
import org.springframework.boot.logging.LoggingSystem;

import static org.assertj.core.api.Assertions.assertThat;
import static org.mockito.BDDMockito.given;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;


public class LoggersEndpointTests {

	private final LoggingSystem loggingSystem = mock(LoggingSystem.class);

	@Test
	@SuppressWarnings("unchecked")
	public void loggersShouldReturnLoggerConfigurations() {
		given(this.loggingSystem.getLoggerConfigurations()).willReturn(Collections
				.singletonList(new LoggerConfiguration("ROOT", null, LogLevel.DEBUG)));
		given(this.loggingSystem.getSupportedLogLevels())
				.willReturn(EnumSet.allOf(LogLevel.class));
		Map<String, Object> result = new LoggersEndpoint(this.loggingSystem).loggers();
		Map<String, LoggerLevels> loggers = (Map<String, LoggerLevels>) result
				.get("loggers");
		Set<LogLevel> levels = (Set<LogLevel>) result.get("levels");
		LoggerLevels rootLevels = loggers.get("ROOT");
		assertThat(rootLevels.getConfiguredLevel()).isNull();
		assertThat(rootLevels.getEffectiveLevel()).isEqualTo("DEBUG");
		assertThat(levels).containsExactly(LogLevel.OFF, LogLevel.FATAL, LogLevel.ERROR,
				LogLevel.WARN, LogLevel.INFO, LogLevel.DEBUG, LogLevel.TRACE);
	}

	@Test
	public void loggerLevelsWhenNameSpecifiedShouldReturnLevels() {
		given(this.loggingSystem.getLoggerConfiguration("ROOT"))
				.willReturn(new LoggerConfiguration("ROOT", null, LogLevel.DEBUG));
		LoggerLevels levels = new LoggersEndpoint(this.loggingSystem)
				.loggerLevels("ROOT");
		assertThat(levels.getConfiguredLevel()).isNull();
		assertThat(levels.getEffectiveLevel()).isEqualTo("DEBUG");
	}

	@Test
	public void configureLogLevelShouldSetLevelOnLoggingSystem() {
		new LoggersEndpoint(this.loggingSystem).configureLogLevel("ROOT", LogLevel.DEBUG);
		verify(this.loggingSystem).setLogLevel("ROOT", LogLevel.DEBUG);
	}

	@Test
	public void configureLogLevelWithNullSetsLevelOnLoggingSystemToNull() {
		new LoggersEndpoint(this.loggingSystem).configureLogLevel("ROOT", null);
		verify(this.loggingSystem).setLogLevel("ROOT", null);
	}

}
