

package org.springframework.boot.loader;

import java.io.File;
import java.net.URL;

import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;

import org.springframework.boot.loader.jar.JarFile;

import static org.assertj.core.api.Assertions.assertThat;


@SuppressWarnings("resource")
public class LaunchedURLClassLoaderTests {

	@Rule
	public TemporaryFolder temporaryFolder = new TemporaryFolder();

	@Test
	public void resolveResourceFromArchive() throws Exception {
		LaunchedURLClassLoader loader = new LaunchedURLClassLoader(
				new URL[] { new URL("jar:file:src/test/resources/jars/app.jar!/") },
				getClass().getClassLoader());
		assertThat(loader.getResource("demo/Application.java")).isNotNull();
	}

	@Test
	public void resolveResourcesFromArchive() throws Exception {
		LaunchedURLClassLoader loader = new LaunchedURLClassLoader(
				new URL[] { new URL("jar:file:src/test/resources/jars/app.jar!/") },
				getClass().getClassLoader());
		assertThat(loader.getResources("demo/Application.java").hasMoreElements())
				.isTrue();
	}

	@Test
	public void resolveRootPathFromArchive() throws Exception {
		LaunchedURLClassLoader loader = new LaunchedURLClassLoader(
				new URL[] { new URL("jar:file:src/test/resources/jars/app.jar!/") },
				getClass().getClassLoader());
		assertThat(loader.getResource("")).isNotNull();
	}

	@Test
	public void resolveRootResourcesFromArchive() throws Exception {
		LaunchedURLClassLoader loader = new LaunchedURLClassLoader(
				new URL[] { new URL("jar:file:src/test/resources/jars/app.jar!/") },
				getClass().getClassLoader());
		assertThat(loader.getResources("").hasMoreElements()).isTrue();
	}

	@Test
	public void resolveFromNested() throws Exception {
		File file = this.temporaryFolder.newFile();
		TestJarCreator.createTestJar(file);
		JarFile jarFile = new JarFile(file);
		URL url = jarFile.getUrl();
		LaunchedURLClassLoader loader = new LaunchedURLClassLoader(new URL[] { url },
				null);
		URL resource = loader.getResource("nested.jar!/3.dat");
		assertThat(resource.toString()).isEqualTo(url + "nested.jar!/3.dat");
		assertThat(resource.openConnection().getInputStream().read()).isEqualTo(3);
	}

	@Test
	public void resolveFromNestedWhileThreadIsInterrupted() throws Exception {
		File file = this.temporaryFolder.newFile();
		TestJarCreator.createTestJar(file);
		JarFile jarFile = new JarFile(file);
		URL url = jarFile.getUrl();
		LaunchedURLClassLoader loader = new LaunchedURLClassLoader(new URL[] { url },
				null);
		try {
			Thread.currentThread().interrupt();
			URL resource = loader.getResource("nested.jar!/3.dat");
			assertThat(resource.toString()).isEqualTo(url + "nested.jar!/3.dat");
			assertThat(resource.openConnection().getInputStream().read()).isEqualTo(3);
		}
		finally {
			Thread.interrupted();
		}
	}

}
