

package org.springframework.boot.actuate.logging;

import java.io.File;
import java.io.IOException;
import java.nio.charset.StandardCharsets;

import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;

import org.springframework.core.io.Resource;
import org.springframework.mock.env.MockEnvironment;
import org.springframework.util.FileCopyUtils;
import org.springframework.util.StreamUtils;

import static org.assertj.core.api.Assertions.assertThat;


public class LogFileWebEndpointTests {

	@Rule
	public TemporaryFolder temp = new TemporaryFolder();

	private final MockEnvironment environment = new MockEnvironment();

	private final LogFileWebEndpoint endpoint = new LogFileWebEndpoint(this.environment);

	private File logFile;

	@Before
	public void before() throws IOException {
		this.logFile = this.temp.newFile();
		FileCopyUtils.copy("--TEST--".getBytes(), this.logFile);
	}

	@Test
	public void nullResponseWithoutLogFile() {
		assertThat(this.endpoint.logFile()).isNull();
	}

	@Test
	public void nullResponseWithMissingLogFile() {
		this.environment.setProperty("logging.file", "no_test.log");
		assertThat(this.endpoint.logFile()).isNull();
	}

	@Test
	public void resourceResponseWithLogFile() throws Exception {
		this.environment.setProperty("logging.file", this.logFile.getAbsolutePath());
		Resource resource = this.endpoint.logFile();
		assertThat(resource).isNotNull();
		assertThat(StreamUtils.copyToString(resource.getInputStream(),
				StandardCharsets.UTF_8)).isEqualTo("--TEST--");
	}

	@Test
	public void resourceResponseWithExternalLogFile() throws Exception {
		LogFileWebEndpoint endpoint = new LogFileWebEndpoint(this.environment,
				this.logFile);
		Resource resource = endpoint.logFile();
		assertThat(resource).isNotNull();
		assertThat(StreamUtils.copyToString(resource.getInputStream(),
				StandardCharsets.UTF_8)).isEqualTo("--TEST--");
	}

}
