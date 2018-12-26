

package org.springframework.boot.devtools.tests;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.jar.Attributes;
import java.util.jar.JarOutputStream;
import java.util.jar.Manifest;
import java.util.zip.ZipEntry;

import org.springframework.util.FileSystemUtils;
import org.springframework.util.StreamUtils;
import org.springframework.util.StringUtils;


public class JarFileRemoteApplicationLauncher extends RemoteApplicationLauncher {

	@Override
	protected String createApplicationClassPath() throws Exception {
		File appDirectory = new File("target/app");
		if (appDirectory.isDirectory()
				&& !FileSystemUtils.deleteRecursively(appDirectory.toPath())) {
			throw new IllegalStateException(
					"Failed to delete '" + appDirectory.getAbsolutePath() + "'");
		}
		appDirectory.mkdirs();
		Manifest manifest = new Manifest();
		manifest.getMainAttributes().put(Attributes.Name.MANIFEST_VERSION, "1.0");
		JarOutputStream output = new JarOutputStream(
				new FileOutputStream(new File(appDirectory, "app.jar")), manifest);
		FileSystemUtils.copyRecursively(new File("target/test-classes/com"),
				new File("target/app/com"));
		addToJar(output, new File("target/app/"), new File("target/app/"));
		output.close();
		List<String> entries = new ArrayList<>();
		entries.add("target/app/app.jar");
		for (File jar : new File("target/dependencies").listFiles()) {
			entries.add(jar.getAbsolutePath());
		}
		String classpath = StringUtils.collectionToDelimitedString(entries,
				File.pathSeparator);
		return classpath;
	}

	private void addToJar(JarOutputStream output, File root, File current)
			throws IOException {
		for (File file : current.listFiles()) {
			if (file.isDirectory()) {
				addToJar(output, root, file);
			}
			output.putNextEntry(new ZipEntry(
					file.getAbsolutePath().substring(root.getAbsolutePath().length() + 1)
							.replace("\\", "/") + (file.isDirectory() ? "/" : "")));
			if (file.isFile()) {
				try (FileInputStream input = new FileInputStream(file)) {
					StreamUtils.copy(input, output);
				}
			}
			output.closeEntry();
		}
	}

	@Override
	public String toString() {
		return "jar file remote";
	}

}
