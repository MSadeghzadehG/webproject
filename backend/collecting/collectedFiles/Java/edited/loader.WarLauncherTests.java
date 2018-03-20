

package org.springframework.boot.loader;

import java.io.File;
import java.net.URL;
import java.util.List;

import org.junit.Test;

import org.springframework.boot.loader.archive.Archive;
import org.springframework.boot.loader.archive.ExplodedArchive;
import org.springframework.boot.loader.archive.JarFileArchive;

import static org.assertj.core.api.Assertions.assertThat;


public class WarLauncherTests extends AbstractExecutableArchiveLauncherTests {

	@Test
	public void explodedWarHasOnlyWebInfClassesAndContentsOfWebInfLibOnClasspath()
			throws Exception {
		File explodedRoot = explode(createJarArchive("archive.war", "WEB-INF"));
		WarLauncher launcher = new WarLauncher(new ExplodedArchive(explodedRoot, true));
		List<Archive> archives = launcher.getClassPathArchives();
		assertThat(archives).hasSize(2);
		assertThat(getUrls(archives)).containsOnly(
				new File(explodedRoot, "WEB-INF/classes").toURI().toURL(),
				new URL("jar:"
						+ new File(explodedRoot, "WEB-INF/lib/foo.jar").toURI().toURL()
						+ "!/"));
	}

	@Test
	public void archivedWarHasOnlyWebInfClassesAndContentsOWebInfLibOnClasspath()
			throws Exception {
		File jarRoot = createJarArchive("archive.war", "WEB-INF");
		WarLauncher launcher = new WarLauncher(new JarFileArchive(jarRoot));
		List<Archive> archives = launcher.getClassPathArchives();
		assertThat(archives).hasSize(2);
		assertThat(getUrls(archives)).containsOnly(
				new URL("jar:" + jarRoot.toURI().toURL() + "!/WEB-INF/classes!/"),
				new URL("jar:" + jarRoot.toURI().toURL() + "!/WEB-INF/lib/foo.jar!/"));
	}

}
