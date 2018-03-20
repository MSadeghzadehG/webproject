

package com.badlogic.gdx.backends.lwjgl3;

import java.io.File;

import com.badlogic.gdx.Files;
import com.badlogic.gdx.files.FileHandle;


public final class Lwjgl3Files implements Files {
	static public final String externalPath = System.getProperty("user.home") + File.separator;
	static public final String localPath = new File("").getAbsolutePath() + File.separator;
	
	@Override
	public FileHandle getFileHandle (String fileName, FileType type) {
		return new Lwjgl3FileHandle(fileName, type);
	}

	@Override
	public FileHandle classpath (String path) {
		return new Lwjgl3FileHandle(path, FileType.Classpath);
	}

	@Override
	public FileHandle internal (String path) {
		return new Lwjgl3FileHandle(path, FileType.Internal);
	}

	@Override
	public FileHandle external (String path) {
		return new Lwjgl3FileHandle(path, FileType.External);
	}

	@Override
	public FileHandle absolute (String path) {
		return new Lwjgl3FileHandle(path, FileType.Absolute);
	}

	@Override
	public FileHandle local (String path) {
		return new Lwjgl3FileHandle(path, FileType.Local);
	}

	@Override
	public String getExternalStoragePath () {
		return externalPath;
	}

	@Override
	public boolean isExternalStorageAvailable () {
		return true;
	}

	@Override
	public String getLocalStoragePath () {
		return localPath;
	}

	@Override
	public boolean isLocalStorageAvailable () {
		return true;
	}
}
