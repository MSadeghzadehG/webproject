

package org.springframework.boot.devtools.settings;

import java.io.File;
import java.io.IOException;
import java.net.URL;

import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;

import static org.assertj.core.api.Assertions.assertThat;


public class DevToolsSettingsTests {

	@Rule
	public TemporaryFolder temporaryFolder = new TemporaryFolder();

	private static final String ROOT = DevToolsSettingsTests.class.getPackage().getName()
			.replace('.', '/') + "/";

	@Test
	public void includePatterns() throws Exception {
		DevToolsSettings settings = DevToolsSettings
				.load(ROOT + "spring-devtools-include.properties");
		assertThat(settings.isRestartInclude(new URL("file:		assertThat(settings.isRestartInclude(new URL("file:		assertThat(settings.isRestartInclude(new URL("file:	}

	@Test
	public void excludePatterns() throws Exception {
		DevToolsSettings settings = DevToolsSettings
				.load(ROOT + "spring-devtools-exclude.properties");
		assertThat(settings.isRestartExclude(new URL("file:		assertThat(settings.isRestartExclude(new URL("file:		assertThat(settings.isRestartExclude(new URL("file:	}

	@Test
	public void defaultIncludePatterns() throws Exception {
		DevToolsSettings settings = DevToolsSettings.get();
		assertThat(settings.isRestartExclude(makeUrl("spring-boot"))).isTrue();
		assertThat(settings.isRestartExclude(makeUrl("spring-boot-autoconfigure")))
				.isTrue();
		assertThat(settings.isRestartExclude(makeUrl("spring-boot-actuator"))).isTrue();
		assertThat(settings.isRestartExclude(makeUrl("spring-boot-starter"))).isTrue();
		assertThat(settings.isRestartExclude(makeUrl("spring-boot-starter-some-thing")))
				.isTrue();
	}

	private URL makeUrl(String name) throws IOException {
		File file = this.temporaryFolder.newFolder();
		file = new File(file, name);
		file = new File(file, "target");
		file = new File(file, "classes");
		file.mkdirs();
		return file.toURI().toURL();
	}

}
