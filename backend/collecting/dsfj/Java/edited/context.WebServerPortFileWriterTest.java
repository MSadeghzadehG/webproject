

package org.springframework.boot.web.context;

import java.io.File;
import java.io.FileReader;
import java.util.HashSet;
import java.util.Locale;
import java.util.Set;

import org.junit.After;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;

import org.springframework.boot.web.server.WebServer;
import org.springframework.util.FileCopyUtils;
import org.springframework.util.StringUtils;

import static org.assertj.core.api.Assertions.assertThat;
import static org.mockito.BDDMockito.given;
import static org.mockito.Mockito.mock;


public class WebServerPortFileWriterTest {

	@Rule
	public final TemporaryFolder temporaryFolder = new TemporaryFolder();

	@Before
	@After
	public void reset() {
		System.clearProperty("PORTFILE");
	}

	@Test
	public void createPortFile() throws Exception {
		File file = this.temporaryFolder.newFile();
		WebServerPortFileWriter listener = new WebServerPortFileWriter(file);
		listener.onApplicationEvent(mockEvent("", 8080));
		assertThat(FileCopyUtils.copyToString(new FileReader(file))).isEqualTo("8080");
	}

	@Test
	public void overridePortFileWithDefault() throws Exception {
		System.setProperty("PORTFILE", this.temporaryFolder.newFile().getAbsolutePath());
		WebServerPortFileWriter listener = new WebServerPortFileWriter();
		listener.onApplicationEvent(mockEvent("", 8080));
		FileReader reader = new FileReader(System.getProperty("PORTFILE"));
		assertThat(FileCopyUtils.copyToString(reader)).isEqualTo("8080");
	}

	@Test
	public void overridePortFileWithExplicitFile() throws Exception {
		File file = this.temporaryFolder.newFile();
		System.setProperty("PORTFILE", this.temporaryFolder.newFile().getAbsolutePath());
		WebServerPortFileWriter listener = new WebServerPortFileWriter(file);
		listener.onApplicationEvent(mockEvent("", 8080));
		FileReader reader = new FileReader(System.getProperty("PORTFILE"));
		assertThat(FileCopyUtils.copyToString(reader)).isEqualTo("8080");
	}

	@Test
	public void createManagementPortFile() throws Exception {
		File file = this.temporaryFolder.newFile();
		WebServerPortFileWriter listener = new WebServerPortFileWriter(file);
		listener.onApplicationEvent(mockEvent("", 8080));
		listener.onApplicationEvent(mockEvent("management", 9090));
		assertThat(FileCopyUtils.copyToString(new FileReader(file))).isEqualTo("8080");
		String managementFile = file.getName();
		managementFile = managementFile.substring(0, managementFile.length()
				- StringUtils.getFilenameExtension(managementFile).length() - 1);
		managementFile = managementFile + "-management."
				+ StringUtils.getFilenameExtension(file.getName());
		FileReader reader = new FileReader(
				new File(file.getParentFile(), managementFile));
		assertThat(FileCopyUtils.copyToString(reader)).isEqualTo("9090");
		assertThat(collectFileNames(file.getParentFile())).contains(managementFile);
	}

	@Test
	public void createUpperCaseManagementPortFile() throws Exception {
		File file = this.temporaryFolder.newFile();
		file = new File(file.getParentFile(), file.getName().toUpperCase(Locale.ENGLISH));
		WebServerPortFileWriter listener = new WebServerPortFileWriter(file);
		listener.onApplicationEvent(mockEvent("management", 9090));
		String managementFile = file.getName();
		managementFile = managementFile.substring(0, managementFile.length()
				- StringUtils.getFilenameExtension(managementFile).length() - 1);
		managementFile = managementFile + "-MANAGEMENT."
				+ StringUtils.getFilenameExtension(file.getName());
		FileReader reader = new FileReader(
				new File(file.getParentFile(), managementFile));
		assertThat(FileCopyUtils.copyToString(reader)).isEqualTo("9090");
		assertThat(collectFileNames(file.getParentFile())).contains(managementFile);
	}

	private WebServerInitializedEvent mockEvent(String namespace, int port) {
		WebServer webServer = mock(WebServer.class);
		given(webServer.getPort()).willReturn(port);
		WebServerApplicationContext applicationContext = mock(
				WebServerApplicationContext.class);
		given(applicationContext.getServerNamespace()).willReturn(namespace);
		given(applicationContext.getWebServer()).willReturn(webServer);
		WebServerInitializedEvent event = mock(WebServerInitializedEvent.class);
		given(event.getApplicationContext()).willReturn(applicationContext);
		given(event.getWebServer()).willReturn(webServer);
		return event;
	}

	private Set<String> collectFileNames(File directory) {
		Set<String> names = new HashSet<>();
		if (directory.isDirectory()) {
			for (File file : directory.listFiles()) {
				names.add(file.getName());
			}
		}
		return names;
	}

}
