

package org.springframework.boot.devtools.filewatch;

import java.io.File;
import java.util.Collections;
import java.util.Iterator;
import java.util.Set;


public final class ChangedFiles implements Iterable<ChangedFile> {

	private final File sourceFolder;

	private final Set<ChangedFile> files;

	public ChangedFiles(File sourceFolder, Set<ChangedFile> files) {
		this.sourceFolder = sourceFolder;
		this.files = Collections.unmodifiableSet(files);
	}

	
	public File getSourceFolder() {
		return this.sourceFolder;
	}

	@Override
	public Iterator<ChangedFile> iterator() {
		return getFiles().iterator();
	}

	
	public Set<ChangedFile> getFiles() {
		return this.files;
	}

	@Override
	public int hashCode() {
		return this.files.hashCode();
	}

	@Override
	public boolean equals(Object obj) {
		if (obj == null) {
			return false;
		}
		if (obj == this) {
			return true;
		}
		if (obj instanceof ChangedFiles) {
			ChangedFiles other = (ChangedFiles) obj;
			return this.sourceFolder.equals(other.sourceFolder)
					&& this.files.equals(other.files);
		}
		return super.equals(obj);
	}

	@Override
	public String toString() {
		return this.sourceFolder + " " + this.files;
	}

}
