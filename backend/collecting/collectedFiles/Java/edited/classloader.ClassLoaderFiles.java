

package org.springframework.boot.devtools.restart.classloader;

import java.io.Serializable;
import java.util.Collection;
import java.util.Collections;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;

import javax.management.loading.ClassLoaderRepository;

import org.springframework.util.Assert;


public class ClassLoaderFiles implements ClassLoaderFileRepository, Serializable {

	private static final long serialVersionUID = 1;

	private final Map<String, SourceFolder> sourceFolders;

	
	public ClassLoaderFiles() {
		this.sourceFolders = new LinkedHashMap<>();
	}

	
	public ClassLoaderFiles(ClassLoaderFiles classLoaderFiles) {
		Assert.notNull(classLoaderFiles, "ClassLoaderFiles must not be null");
		this.sourceFolders = new LinkedHashMap<>(classLoaderFiles.sourceFolders);
	}

	
	public void addAll(ClassLoaderFiles files) {
		Assert.notNull(files, "Files must not be null");
		for (SourceFolder folder : files.getSourceFolders()) {
			for (Map.Entry<String, ClassLoaderFile> entry : folder.getFilesEntrySet()) {
				addFile(folder.getName(), entry.getKey(), entry.getValue());
			}
		}
	}

	
	public void addFile(String name, ClassLoaderFile file) {
		addFile("", name, file);
	}

	
	public void addFile(String sourceFolder, String name, ClassLoaderFile file) {
		Assert.notNull(sourceFolder, "SourceFolder must not be null");
		Assert.notNull(name, "Name must not be null");
		Assert.notNull(file, "File must not be null");
		removeAll(name);
		getOrCreateSourceFolder(sourceFolder).add(name, file);
	}

	private void removeAll(String name) {
		for (SourceFolder sourceFolder : this.sourceFolders.values()) {
			sourceFolder.remove(name);
		}
	}

	
	protected final SourceFolder getOrCreateSourceFolder(String name) {
		SourceFolder sourceFolder = this.sourceFolders.get(name);
		if (sourceFolder == null) {
			sourceFolder = new SourceFolder(name);
			this.sourceFolders.put(name, sourceFolder);
		}
		return sourceFolder;
	}

	
	public Collection<SourceFolder> getSourceFolders() {
		return Collections.unmodifiableCollection(this.sourceFolders.values());
	}

	
	public int size() {
		int size = 0;
		for (SourceFolder sourceFolder : this.sourceFolders.values()) {
			size += sourceFolder.getFiles().size();
		}
		return size;
	}

	@Override
	public ClassLoaderFile getFile(String name) {
		for (SourceFolder sourceFolder : this.sourceFolders.values()) {
			ClassLoaderFile file = sourceFolder.get(name);
			if (file != null) {
				return file;
			}
		}
		return null;
	}

	
	public static class SourceFolder implements Serializable {

		private static final long serialVersionUID = 1;

		private final String name;

		private final Map<String, ClassLoaderFile> files = new LinkedHashMap<>();

		SourceFolder(String name) {
			this.name = name;
		}

		public Set<Entry<String, ClassLoaderFile>> getFilesEntrySet() {
			return this.files.entrySet();
		}

		protected final void add(String name, ClassLoaderFile file) {
			this.files.put(name, file);
		}

		protected final void remove(String name) {
			this.files.remove(name);
		}

		protected final ClassLoaderFile get(String name) {
			return this.files.get(name);
		}

		
		public String getName() {
			return this.name;
		}

		
		public Collection<ClassLoaderFile> getFiles() {
			return Collections.unmodifiableCollection(this.files.values());
		}

	}

}
