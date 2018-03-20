

package com.badlogic.gdx.backends.android;

import java.io.IOException;

import android.app.Activity;
import android.app.Fragment;
import android.content.Context;
import android.content.res.AssetManager;
import android.os.Environment;

import com.badlogic.gdx.Files;
import com.badlogic.gdx.Gdx;
import com.badlogic.gdx.files.FileHandle;
import com.badlogic.gdx.utils.GdxRuntimeException;


public class AndroidFiles implements Files {
	protected final String sdcard = Environment.getExternalStorageDirectory().getAbsolutePath() + "/";
	protected final String localpath;

	protected final AssetManager assets;
	private ZipResourceFile expansionFile = null;

	public AndroidFiles (AssetManager assets) {
		this.assets = assets;
		localpath = sdcard;
	}

	public AndroidFiles (AssetManager assets, String localpath) {
		this.assets = assets;
		this.localpath = localpath.endsWith("/") ? localpath : localpath + "/";
	}

	@Override
	public FileHandle getFileHandle (String path, FileType type) {
		FileHandle handle = new AndroidFileHandle(type == FileType.Internal ? assets : null, path, type);
		if (expansionFile != null && type == FileType.Internal) handle = getZipFileHandleIfExists(handle, path);
		return handle;
	}

	private FileHandle getZipFileHandleIfExists (FileHandle handle, String path) {
		try {
			assets.open(path).close(); 			return handle;
		} catch (Exception ex) {
						FileHandle zipHandle = new AndroidZipFileHandle(path);
			if (!zipHandle.isDirectory())
				return zipHandle;
			else if (zipHandle.exists()) return zipHandle;
		}
		return handle;
	}

	@Override
	public FileHandle classpath (String path) {
		return new AndroidFileHandle(null, path, FileType.Classpath);
	}

	@Override
	public FileHandle internal (String path) {
		FileHandle handle = new AndroidFileHandle(assets, path, FileType.Internal);
		if (expansionFile != null) handle = getZipFileHandleIfExists(handle, path);
		return handle;
	}

	@Override
	public FileHandle external (String path) {
		return new AndroidFileHandle(null, path, FileType.External);
	}

	@Override
	public FileHandle absolute (String path) {
		return new AndroidFileHandle(null, path, FileType.Absolute);
	}

	@Override
	public FileHandle local (String path) {
		return new AndroidFileHandle(null, path, FileType.Local);
	}

	@Override
	public String getExternalStoragePath () {
		return sdcard;
	}

	@Override
	public boolean isExternalStorageAvailable () {
		return Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED);
	}

	@Override
	public String getLocalStoragePath () {
		return localpath;
	}

	@Override
	public boolean isLocalStorageAvailable () {
		return true;
	}

	
	public boolean setAPKExpansion(int mainVersion, int patchVersion) {
		try {
			Context context;
			if (Gdx.app instanceof Activity) {
				context = ((Activity) Gdx.app).getBaseContext();
			} else if (Gdx.app instanceof Fragment) {
				context = ((Fragment) Gdx.app).getActivity().getBaseContext();
			} else {
				throw new GdxRuntimeException("APK expansion not supported for application type");
			}
			expansionFile = APKExpansionSupport.getAPKExpansionZipFile(
					context,
					mainVersion, patchVersion);
		} catch (IOException ex) {
			throw new GdxRuntimeException("APK expansion main version " + mainVersion + " or patch version " + patchVersion + " couldn't be opened!");
		}
		return expansionFile != null;
	}

	
	public ZipResourceFile getExpansionFile() {
		return expansionFile;
	}
}
