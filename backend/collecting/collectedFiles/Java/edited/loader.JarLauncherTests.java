

package org.springframework.boot.loader;

import java.io.File;
import java.net.URL;
import java.util.List;

import org.junit.Test;

import org.springframework.boot.loader.archive.Archive;
import org.springframework.boot.loader.archive.ExplodedArchive;
import org.springframework.boot.loader.archive.JarFileArchive;

import static org.assertj.core.api.Assertions.assertThat;


public class JarLauncherTests extends AbstractExecutableArchiveLauncherTests {

	@Test
	public void explodedJarHasOnlyBootInfClassesAndContentsOfBootInfLibOnClasspath()
			throws Exception {
		File explodedRoot = explode(createJarArchive("archive.jar", "BOOT-INF"));
		JarLauncher launcher = new JarLauncher(new ExplodedArchive(explodedRoot, true));
		List<Archive> archives = launcher.getClassPathArchives();
		assertThat(archives).hasSize(2);
		assertThat(getUrls(archives)).containsOnly(
				new File(explodedRoot, "BOOT-INF/classes").toURI().toURL(),
				new URL("jar:"
						+ new File(explodedRoot, "BOOT-INF/lib/foo.jar").toURI().toURL()
						+ "!/"));
	}

	@Test
	public void archivedJarHasOnlyBootInfClassesAndContentsOfBootInfLibOnClasspath()
			throws Exception {
		File jarRoot = createJarArchive("archive.jar", "BOOT-INF");
		JarLauncher launcher = new JarLauncher(new JarFileArchive(jarRoot));
		List<Archive> archives = launcher.getClassPathArchives();
		assertThat(archives).hasSize(2);
		assertThat(getUrls(archives)).containsOnly(
				new URL("jar:" + jarRoot.toURI().toURL() + "!/BOOT-INF/classes!/"),
				new URL("jar:" + jarRoot.toURI().toURL() + "!/BOOT-INF/lib/foo.jar!/"));
	}

}
