

package org.springframework.boot.web.embedded.undertow;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.jar.JarOutputStream;
import java.util.zip.ZipEntry;

import io.undertow.server.handlers.resource.Resource;
import io.undertow.server.handlers.resource.ResourceManager;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;

import static org.assertj.core.api.Assertions.assertThat;


public class JarResourceManagerTests {

	@Rule
	public TemporaryFolder temp = new TemporaryFolder();

	private ResourceManager resourceManager;

	@Before
	public void createJar() throws IOException {
		File jar = this.temp.newFile();
		JarOutputStream out = new JarOutputStream(new FileOutputStream(jar));
		out.putNextEntry(new ZipEntry("hello.txt"));
		out.write("hello".getBytes());
		out.close();
		this.resourceManager = new JarResourceManager(jar);
	}

	@Test
	public void emptyPathIsHandledCorrectly() throws IOException {
		Resource resource = this.resourceManager.getResource("");
		assertThat(resource).isNotNull();
		assertThat(resource.isDirectory()).isTrue();
	}

	@Test
	public void rootPathIsHandledCorrectly() throws IOException {
		Resource resource = this.resourceManager.getResource("/");
		assertThat(resource).isNotNull();
		assertThat(resource.isDirectory()).isTrue();
	}

	@Test
	public void resourceIsFoundInJarFile() throws IOException {
		Resource resource = this.resourceManager.getResource("/hello.txt");
		assertThat(resource).isNotNull();
		assertThat(resource.isDirectory()).isFalse();
		assertThat(resource.getContentLength()).isEqualTo(5);
	}

	@Test
	public void resourceIsFoundInJarFileWithoutLeadingSlash() throws IOException {
		Resource resource = this.resourceManager.getResource("hello.txt");
		assertThat(resource).isNotNull();
		assertThat(resource.isDirectory()).isFalse();
		assertThat(resource.getContentLength()).isEqualTo(5);
	}

}
