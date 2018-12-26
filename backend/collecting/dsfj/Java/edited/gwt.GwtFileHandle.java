

package com.badlogic.gdx.backends.gwt;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileFilter;
import java.io.FilenameFilter;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.Reader;
import java.io.UnsupportedEncodingException;
import java.io.Writer;

import com.badlogic.gdx.Files.FileType;
import com.badlogic.gdx.Gdx;
import com.badlogic.gdx.backends.gwt.preloader.Preloader;
import com.badlogic.gdx.files.FileHandle;
import com.badlogic.gdx.utils.GdxRuntimeException;

public class GwtFileHandle extends FileHandle {
	public final Preloader preloader;
	private final String file;
	private final FileType type;

	public GwtFileHandle (Preloader preloader, String fileName, FileType type) {
		if (type != FileType.Internal && type != FileType.Classpath)
			throw new GdxRuntimeException("FileType '" + type + "' Not supported in GWT backend");
		this.preloader = preloader;
		this.file = fixSlashes(fileName);
		this.type = type;
	}

	public GwtFileHandle (String path) {
		this.type = FileType.Internal;
		this.preloader = ((GwtApplication)Gdx.app).getPreloader();
		this.file = fixSlashes(path);
	}

	public String path () {
		return file;
	}

	public String name () {
		int index = file.lastIndexOf('/');
		if (index < 0) return file;
		return file.substring(index + 1);
	}

	public String extension () {
		String name = name();
		int dotIndex = name.lastIndexOf('.');
		if (dotIndex == -1) return "";
		return name.substring(dotIndex + 1);
	}

	public String nameWithoutExtension () {
		String name = name();
		int dotIndex = name.lastIndexOf('.');
		if (dotIndex == -1) return name;
		return name.substring(0, dotIndex);
	}

	
	public String pathWithoutExtension () {
		String path = file;
		int dotIndex = path.lastIndexOf('.');
		if (dotIndex == -1) return path;
		return path.substring(0, dotIndex);
	}

	public FileType type () {
		return type;
	}

	
	public File file () {
		throw new GdxRuntimeException("Not supported in GWT backend");
	}

	
	public InputStream read () {
		InputStream in = preloader.read(file);
		if (in == null) throw new GdxRuntimeException(file + " does not exist");
		return in;
	}

	
	public BufferedInputStream read (int bufferSize) {
		return new BufferedInputStream(read(), bufferSize);
	}

	
	public Reader reader () {
		return new InputStreamReader(read());
	}

	
	public Reader reader (String charset) {
		try {
			return new InputStreamReader(read(), charset);
		} catch (UnsupportedEncodingException e) {
			throw new GdxRuntimeException("Encoding '" + charset + "' not supported", e);
		}
	}

	
	public BufferedReader reader (int bufferSize) {
		return new BufferedReader(reader(), bufferSize);
	}

	
	public BufferedReader reader (int bufferSize, String charset) {
		return new BufferedReader(reader(charset), bufferSize);
	}

	
	public String readString () {
		return readString(null);
	}

	
	public String readString (String charset) {
		if (preloader.isText(file)) return preloader.texts.get(file);
		try {
			return new String(readBytes(), "UTF-8");
		} catch (UnsupportedEncodingException e) {
			return null;
		}
	}

	
	public byte[] readBytes () {
		int length = (int)length();
		if (length == 0) length = 512;
		byte[] buffer = new byte[length];
		int position = 0;
		InputStream input = read();
		try {
			while (true) {
				int count = input.read(buffer, position, buffer.length - position);
				if (count == -1) break;
				position += count;
				if (position == buffer.length) {
										byte[] newBuffer = new byte[buffer.length * 2];
					System.arraycopy(buffer, 0, newBuffer, 0, position);
					buffer = newBuffer;
				}
			}
		} catch (IOException ex) {
			throw new GdxRuntimeException("Error reading file: " + this, ex);
		} finally {
			try {
				if (input != null) input.close();
			} catch (IOException ignored) {
			}
		}
		if (position < buffer.length) {
						byte[] newBuffer = new byte[position];
			System.arraycopy(buffer, 0, newBuffer, 0, position);
			buffer = newBuffer;
		}
		return buffer;
	}

	
	public int readBytes (byte[] bytes, int offset, int size) {
		InputStream input = read();
		int position = 0;
		try {
			while (true) {
				int count = input.read(bytes, offset + position, size - position);
				if (count <= 0) break;
				position += count;
			}
		} catch (IOException ex) {
			throw new GdxRuntimeException("Error reading file: " + this, ex);
		} finally {
			try {
				if (input != null) input.close();
			} catch (IOException ignored) {
			}
		}
		return position - offset;
	}

	
	public OutputStream write (boolean append) {
		throw new GdxRuntimeException("Cannot write to files in GWT backend");
	}

	
	public void write (InputStream input, boolean append) {
		throw new GdxRuntimeException("Cannot write to files in GWT backend");
	}

	
	public Writer writer (boolean append) {
		return writer(append, null);
	}

	
	public Writer writer (boolean append, String charset) {
		throw new GdxRuntimeException("Cannot write to files in GWT backend");
	}

	
	public void writeString (String string, boolean append) {
		writeString(string, append, null);
	}

	
	public void writeString (String string, boolean append, String charset) {
		throw new GdxRuntimeException("Cannot write to files in GWT backend");
	}

	
	public void writeBytes (byte[] bytes, boolean append) {
		throw new GdxRuntimeException("Cannot write to files in GWT backend");
	}

	
	public void writeBytes (byte[] bytes, int offset, int length, boolean append) {
		throw new GdxRuntimeException("Cannot write to files in GWT backend");
	}

	
	public FileHandle[] list () {
		return preloader.list(file);
	}

	
	public FileHandle[] list (FileFilter filter) {
		return preloader.list(file, filter);
	}

	
	public FileHandle[] list (FilenameFilter filter) {
		return preloader.list(file, filter);
	}

	
	public FileHandle[] list (String suffix) {
		return preloader.list(file, suffix);
	}

	
	public boolean isDirectory () {
		return preloader.isDirectory(file);
	}

	
	public FileHandle child (String name) {
		return new GwtFileHandle(preloader, (file.isEmpty() ? "" : (file + (file.endsWith("/") ? "" : "/"))) + name,
			FileType.Internal);
	}

	public FileHandle parent () {
		int index = file.lastIndexOf("/");
		String dir = "";
		if (index > 0) dir = file.substring(0, index);
		return new GwtFileHandle(preloader, dir, type);
	}

	public FileHandle sibling (String name) {
		return parent().child(fixSlashes(name));
	}

	
	public void mkdirs () {
		throw new GdxRuntimeException("Cannot mkdirs with an internal file: " + file);
	}

	
	public boolean exists () {
		return preloader.contains(file);
	}

	
	public boolean delete () {
		throw new GdxRuntimeException("Cannot delete an internal file: " + file);
	}

	
	public boolean deleteDirectory () {
		throw new GdxRuntimeException("Cannot delete an internal file: " + file);
	}

	
	public void copyTo (FileHandle dest) {
		throw new GdxRuntimeException("Cannot copy to an internal file: " + dest);
	}

	
	public void moveTo (FileHandle dest) {
		throw new GdxRuntimeException("Cannot move an internal file: " + file);
	}

	
	public long length () {
		return preloader.length(file);
	}

	
	public long lastModified () {
		return 0;
	}

	public String toString () {
		return file;
	}

	private static String fixSlashes(String path) {
		path = path.replace('\\', '/');
		if (path.endsWith("/")) {
			path = path.substring(0, path.length() - 1);
		}
		return path;
	}

}
