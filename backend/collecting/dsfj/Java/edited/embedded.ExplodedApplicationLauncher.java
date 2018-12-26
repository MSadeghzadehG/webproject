

package org.springframework.boot.context.embedded;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Arrays;
import java.util.Enumeration;
import java.util.List;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;

import org.springframework.util.FileSystemUtils;
import org.springframework.util.StreamUtils;


class ExplodedApplicationLauncher extends AbstractApplicationLauncher {

	private final File exploded = new File("target/exploded");

	ExplodedApplicationLauncher(ApplicationBuilder applicationBuilder) {
		super(applicationBuilder);
	}

	@Override
	protected File getWorkingDirectory() {
		return this.exploded;
	}

	@Override
	protected String getDescription(String packaging) {
		return "exploded " + packaging;
	}

	@Override
	protected List<String> getArguments(File archive) {
		String mainClass = archive.getName().endsWith(".war")
				? "org.springframework.boot.loader.WarLauncher"
				: "org.springframework.boot.loader.JarLauncher";
		try {
			explodeArchive(archive);
			return Arrays.asList("-cp", this.exploded.getAbsolutePath(), mainClass);
		}
		catch (IOException ex) {
			throw new RuntimeException(ex);
		}
	}

	private void explodeArchive(File archive) throws IOException {
		FileSystemUtils.deleteRecursively(this.exploded);
		JarFile jarFile = new JarFile(archive);
		Enumeration<JarEntry> entries = jarFile.entries();
		while (entries.hasMoreElements()) {
			JarEntry jarEntry = entries.nextElement();
			File extracted = new File(this.exploded, jarEntry.getName());
			if (jarEntry.isDirectory()) {
				extracted.mkdirs();
			}
			else {
				FileOutputStream extractedOutputStream = new FileOutputStream(extracted);
				StreamUtils.copy(jarFile.getInputStream(jarEntry), extractedOutputStream);
				extractedOutputStream.close();
			}
		}
		jarFile.close();
	}

}
