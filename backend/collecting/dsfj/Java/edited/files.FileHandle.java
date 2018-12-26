

package com.badlogic.gdx.files;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileFilter;
import java.io.FilenameFilter;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.Reader;
import java.io.Writer;

import com.badlogic.gdx.Files.FileType;
import com.badlogic.gdx.utils.GdxRuntimeException;

public class FileHandle {
	protected File file;
	protected FileType type;

	protected FileHandle () {
	}

	public FileHandle (String fileName) {
	}

	public FileHandle (File file) {
	}

	protected FileHandle (String fileName, FileType type) {
	}

	protected FileHandle (File file, FileType type) {
	}

	public String path () {
		throw new GdxRuntimeException("Stub");
	}

	public String name () {
		throw new GdxRuntimeException("Stub");
	}

	public String extension () {
		throw new GdxRuntimeException("Stub");
	}

	public String nameWithoutExtension () {
		throw new GdxRuntimeException("Stub");
	}

	
	public String pathWithoutExtension () {
		throw new GdxRuntimeException("Stub");
	}

	public FileType type () {
		throw new GdxRuntimeException("Stub");
	}

	
	public InputStream read () {
		throw new GdxRuntimeException("Stub");
	}

	
	public BufferedInputStream read (int bufferSize) {
		throw new GdxRuntimeException("Stub");
	}

	
	public Reader reader () {
		throw new GdxRuntimeException("Stub");
	}

	
	public Reader reader (String charset) {
		throw new GdxRuntimeException("Stub");
	}

	
	public BufferedReader reader (int bufferSize) {
		throw new GdxRuntimeException("Stub");
	}

	
	public BufferedReader reader (int bufferSize, String charset) {
		throw new GdxRuntimeException("Stub");
	}

	
	public String readString () {
		throw new GdxRuntimeException("Stub");
	}

	
	public String readString (String charset) {
		throw new GdxRuntimeException("Stub");
	}

	
	public byte[] readBytes () {
		throw new GdxRuntimeException("Stub");
	}

	
	public int readBytes (byte[] bytes, int offset, int size) {
		throw new GdxRuntimeException("Stub");
	}

	
	public OutputStream write (boolean append) {
		throw new GdxRuntimeException("Stub");
	}

	
	public OutputStream write (boolean append, int bufferSize) {
		throw new GdxRuntimeException("Stub");
	}

	
	public void write (InputStream input, boolean append) {
		throw new GdxRuntimeException("Stub");
	}

	
	public Writer writer (boolean append) {
		throw new GdxRuntimeException("Stub");
	}

	
	public Writer writer (boolean append, String charset) {
		throw new GdxRuntimeException("Stub");
	}

	
	public void writeString (String string, boolean append) {
		throw new GdxRuntimeException("Stub");
	}

	
	public void writeString (String string, boolean append, String charset) {
		throw new GdxRuntimeException("Stub");
	}

	
	public void writeBytes (byte[] bytes, boolean append) {
		throw new GdxRuntimeException("Stub");
	}

	
	public void writeBytes (byte[] bytes, int offset, int length, boolean append) {
		throw new GdxRuntimeException("Stub");
	}

	
	public FileHandle[] list (FileFilter filter) {
		throw new GdxRuntimeException("Stub");
	}

	
	public FileHandle[] list (FilenameFilter filter) {
		throw new GdxRuntimeException("Stub");
	}

	
	public FileHandle[] list () {
		throw new GdxRuntimeException("Stub");
	}

	
	public FileHandle[] list (String suffix) {
		throw new GdxRuntimeException("Stub");
	}

	
	public boolean isDirectory () {
		throw new GdxRuntimeException("Stub");
	}

	
	public FileHandle child (String name) {
		throw new GdxRuntimeException("Stub");
	}

	public FileHandle parent () {
		throw new GdxRuntimeException("Stub");
	}

	
	public FileHandle sibling (String name) {
		throw new GdxRuntimeException("Stub");
	}

	
	public void mkdirs () {
		throw new GdxRuntimeException("Stub");
	}

	
	public boolean exists () {
		throw new GdxRuntimeException("Stub");
	}

	
	public boolean delete () {
		throw new GdxRuntimeException("Stub");
	}

	
	public boolean deleteDirectory () {
		throw new GdxRuntimeException("Stub");
	}

	
	public void copyTo (FileHandle dest) {
		throw new GdxRuntimeException("Stub");
	}

	
	public void moveTo (FileHandle dest) {
		throw new GdxRuntimeException("Stub");
	}

	
	public long length () {
		throw new GdxRuntimeException("Stub");
	}

	
	public long lastModified () {
		throw new GdxRuntimeException("Stub");
	}

	public String toString () {
		throw new GdxRuntimeException("Stub");
	}
}
