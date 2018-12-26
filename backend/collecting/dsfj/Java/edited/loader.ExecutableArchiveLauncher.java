

package org.springframework.boot.loader;

import java.util.ArrayList;
import java.util.List;
import java.util.jar.JarEntry;
import java.util.jar.Manifest;

import org.springframework.boot.loader.archive.Archive;


public abstract class ExecutableArchiveLauncher extends Launcher {

	private final Archive archive;

	public ExecutableArchiveLauncher() {
		try {
			this.archive = createArchive();
		}
		catch (Exception ex) {
			throw new IllegalStateException(ex);
		}
	}

	protected ExecutableArchiveLauncher(Archive archive) {
		this.archive = archive;
	}

	protected final Archive getArchive() {
		return this.archive;
	}

	@Override
	protected String getMainClass() throws Exception {
		Manifest manifest = this.archive.getManifest();
		String mainClass = null;
		if (manifest != null) {
			mainClass = manifest.getMainAttributes().getValue("Start-Class");
		}
		if (mainClass == null) {
			throw new IllegalStateException(
					"No 'Start-Class' manifest entry specified in " + this);
		}
		return mainClass;
	}

	@Override
	protected List<Archive> getClassPathArchives() throws Exception {
		List<Archive> archives = new ArrayList<>(
				this.archive.getNestedArchives(this::isNestedArchive));
		postProcessClassPathArchives(archives);
		return archives;
	}

	
	protected abstract boolean isNestedArchive(Archive.Entry entry);

	
	protected void postProcessClassPathArchives(List<Archive> archives) throws Exception {
	}

}
