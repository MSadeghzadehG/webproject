

package org.springframework.boot.logging;

import java.io.IOException;

import org.junit.After;
import org.junit.Before;
import org.junit.Rule;
import org.junit.rules.TemporaryFolder;

import org.springframework.util.StringUtils;


public abstract class AbstractLoggingSystemTests {

	private static final String JAVA_IO_TMPDIR = "java.io.tmpdir";

	@Rule
	public TemporaryFolder temp = new TemporaryFolder();

	private String originalTempFolder;

	@Before
	public void configureTempDir() throws IOException {
		this.originalTempFolder = System.getProperty(JAVA_IO_TMPDIR);
		System.setProperty(JAVA_IO_TMPDIR, this.temp.newFolder().getAbsolutePath());
	}

	@After
	public void reinstateTempDir() {
		System.setProperty(JAVA_IO_TMPDIR, this.originalTempFolder);
	}

	@After
	public void clear() {
		System.clearProperty(LoggingSystemProperties.LOG_FILE);
		System.clearProperty(LoggingSystemProperties.PID_KEY);
	}

	protected final String[] getSpringConfigLocations(AbstractLoggingSystem system) {
		return system.getSpringConfigLocations();
	}

	protected final LogFile getLogFile(String file, String path) {
		return getLogFile(file, path, true);
	}

	protected final LogFile getLogFile(String file, String path,
			boolean applyToSystemProperties) {
		LogFile logFile = new LogFile(file, path);
		if (applyToSystemProperties) {
			logFile.applyToSystemProperties();
		}
		return logFile;
	}

	protected final String tmpDir() {
		String path = StringUtils.cleanPath(System.getProperty(JAVA_IO_TMPDIR));
		if (path.endsWith("/")) {
			path = path.substring(0, path.length() - 1);
		}
		return path;
	}

}
