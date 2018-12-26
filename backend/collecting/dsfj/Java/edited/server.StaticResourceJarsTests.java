

package org.springframework.boot.web.servlet.server;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.net.URL;
import java.util.List;
import java.util.function.Consumer;
import java.util.jar.JarEntry;
import java.util.jar.JarOutputStream;

import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;

import static org.assertj.core.api.Assertions.assertThat;


public class StaticResourceJarsTests {

	@Rule
	public final TemporaryFolder temporaryFolder = new TemporaryFolder();

	@Test
	public void includeJarWithStaticResources() throws Exception {
		File jarFile = createResourcesJar("test-resources.jar");
		List<URL> staticResourceJarUrls = new StaticResourceJars()
				.getUrlsFrom(jarFile.toURI().toURL());
		assertThat(staticResourceJarUrls).hasSize(1);
	}

	@Test
	public void includeJarWithStaticResourcesWithUrlEncodedSpaces() throws Exception {
		File jarFile = createResourcesJar("test resources.jar");
		List<URL> staticResourceJarUrls = new StaticResourceJars()
				.getUrlsFrom(jarFile.toURI().toURL());
		assertThat(staticResourceJarUrls).hasSize(1);
	}

	@Test
	public void excludeJarWithoutStaticResources() throws Exception {
		File jarFile = createJar("dependency.jar");
		List<URL> staticResourceJarUrls = new StaticResourceJars()
				.getUrlsFrom(jarFile.toURI().toURL());
		assertThat(staticResourceJarUrls).hasSize(0);
	}

	private File createResourcesJar(String name) throws IOException {
		return createJar(name, (output) -> {
			JarEntry jarEntry = new JarEntry("META-INF/resources");
			try {
				output.putNextEntry(jarEntry);
				output.closeEntry();
			}
			catch (IOException ex) {
				throw new RuntimeException(ex);
			}
		});
	}

	private File createJar(String name) throws IOException {
		return createJar(name, null);
	}

	private File createJar(String name, Consumer<JarOutputStream> customizer)
			throws IOException {
		File jarFile = this.temporaryFolder.newFile(name);
		JarOutputStream jarOutputStream = new JarOutputStream(
				new FileOutputStream(jarFile));
		if (customizer != null) {
			customizer.accept(jarOutputStream);
		}
		jarOutputStream.close();
		return jarFile;
	}

}
