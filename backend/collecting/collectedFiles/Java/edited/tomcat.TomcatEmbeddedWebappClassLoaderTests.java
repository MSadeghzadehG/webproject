

package org.springframework.boot.web.embedded.tomcat;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.net.URL;
import java.net.URLClassLoader;
import java.util.ArrayList;
import java.util.List;
import java.util.jar.JarOutputStream;
import java.util.zip.ZipEntry;

import org.apache.catalina.core.StandardContext;
import org.apache.catalina.loader.ParallelWebappClassLoader;
import org.apache.catalina.webresources.StandardRoot;
import org.apache.catalina.webresources.WarResourceSet;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;

import org.springframework.util.CollectionUtils;

import static org.assertj.core.api.Assertions.assertThat;


public class TomcatEmbeddedWebappClassLoaderTests {

	@Rule
	public TemporaryFolder temp = new TemporaryFolder();

	@Test
	public void getResourceFindsResourceFromParentClassLoader() throws Exception {
		File war = createWar();
		withWebappClassLoader(war,
				(classLoader) -> assertThat(classLoader.getResource("test.txt"))
						.isEqualTo(new URL(webInfClassesUrlString(war) + "test.txt")));
	}

	@Test
	public void getResourcesOnlyFindsResourcesFromParentClassLoader() throws Exception {
		File warFile = createWar();
		withWebappClassLoader(warFile, (classLoader) -> {
			List<URL> urls = new ArrayList<>();
			CollectionUtils.toIterator(classLoader.getResources("test.txt"))
					.forEachRemaining(urls::add);
			assertThat(urls).containsExactly(
					new URL(webInfClassesUrlString(warFile) + "test.txt"));
		});
	}

	private void withWebappClassLoader(File war, ClassLoaderConsumer consumer)
			throws Exception {
		URLClassLoader parent = new URLClassLoader(
				new URL[] { new URL(webInfClassesUrlString(war)) }, null);
		try (ParallelWebappClassLoader classLoader = new TomcatEmbeddedWebappClassLoader(
				parent)) {
			StandardContext context = new StandardContext();
			context.setName("test");
			StandardRoot resources = new StandardRoot();
			resources.setContext(context);
			resources.addJarResources(
					new WarResourceSet(resources, "/", war.getAbsolutePath()));
			resources.start();
			classLoader.setResources(resources);
			classLoader.start();
			consumer.accept(classLoader);
		}
	}

	private String webInfClassesUrlString(File war) {
		return "jar:file:" + war.getAbsolutePath() + "!/WEB-INF/classes/";
	}

	private File createWar() throws IOException {
		File warFile = this.temp.newFile("test.war");
		try (JarOutputStream warOut = new JarOutputStream(
				new FileOutputStream(warFile))) {
			createEntries(warOut, "WEB-INF/", "WEB-INF/classes/",
					"WEB-INF/classes/test.txt");
		}
		return warFile;
	}

	private void createEntries(JarOutputStream out, String... names) throws IOException {
		for (String name : names) {
			out.putNextEntry(new ZipEntry(name));
			out.closeEntry();
		}
	}

	private interface ClassLoaderConsumer {

		void accept(ClassLoader classLoader) throws Exception;

	}

}
