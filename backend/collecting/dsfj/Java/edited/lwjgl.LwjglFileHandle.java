

package com.badlogic.gdx.backends.lwjgl;

import java.io.File;

import com.badlogic.gdx.Files.FileType;
import com.badlogic.gdx.files.FileHandle;
import com.badlogic.gdx.utils.GdxRuntimeException;


public final class LwjglFileHandle extends FileHandle {
	public LwjglFileHandle (String fileName, FileType type) {
		super(fileName, type);
	}

	public LwjglFileHandle (File file, FileType type) {
		super(file, type);
	}

	public FileHandle child (String name) {
		if (file.getPath().length() == 0) return new LwjglFileHandle(new File(name), type);
		return new LwjglFileHandle(new File(file, name), type);
	}

	public FileHandle sibling (String name) {
		if (file.getPath().length() == 0) throw new GdxRuntimeException("Cannot get the sibling of the root.");
		return new LwjglFileHandle(new File(file.getParent(), name), type);
	}

	public FileHandle parent () {
		File parent = file.getParentFile();
		if (parent == null) {
			if (type == FileType.Absolute)
				parent = new File("/");
			else
				parent = new File("");
		}
		return new LwjglFileHandle(parent, type);
	}

	public File file () {
		if (type == FileType.External) return new File(LwjglFiles.externalPath, file.getPath());
		if (type == FileType.Local) return new File(LwjglFiles.localPath, file.getPath());
		return file;
	}
}
