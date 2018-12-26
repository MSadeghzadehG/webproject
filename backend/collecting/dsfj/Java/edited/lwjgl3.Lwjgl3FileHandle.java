

package com.badlogic.gdx.backends.lwjgl3;

import java.io.File;

import com.badlogic.gdx.Files.FileType;
import com.badlogic.gdx.files.FileHandle;
import com.badlogic.gdx.utils.GdxRuntimeException;


public final class Lwjgl3FileHandle extends FileHandle {
	public Lwjgl3FileHandle (String fileName, FileType type) {
		super(fileName, type);
	}

	public Lwjgl3FileHandle (File file, FileType type) {
		super(file, type);
	}

	public FileHandle child (String name) {
		if (file.getPath().length() == 0) return new Lwjgl3FileHandle(new File(name), type);
		return new Lwjgl3FileHandle(new File(file, name), type);
	}

	public FileHandle sibling (String name) {
		if (file.getPath().length() == 0) throw new GdxRuntimeException("Cannot get the sibling of the root.");
		return new Lwjgl3FileHandle(new File(file.getParent(), name), type);
	}

	public FileHandle parent () {
		File parent = file.getParentFile();
		if (parent == null) {
			if (type == FileType.Absolute)
				parent = new File("/");
			else
				parent = new File("");
		}
		return new Lwjgl3FileHandle(parent, type);
	}

	public File file () {
		if (type == FileType.External) return new File(Lwjgl3Files.externalPath, file.getPath());
		if (type == FileType.Local) return new File(Lwjgl3Files.localPath, file.getPath());
		return file;
	}
}
