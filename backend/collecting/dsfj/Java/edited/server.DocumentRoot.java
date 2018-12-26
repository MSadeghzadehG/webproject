

package org.springframework.boot.web.servlet.server;

import java.io.File;
import java.net.JarURLConnection;
import java.net.URL;
import java.net.URLConnection;
import java.security.CodeSource;
import java.util.Arrays;
import java.util.Locale;

import org.apache.commons.logging.Log;


class DocumentRoot {

	private static final String[] COMMON_DOC_ROOTS = { "src/main/webapp", "public",
			"static" };

	private final Log logger;

	private File directory;

	DocumentRoot(Log logger) {
		this.logger = logger;
	}

	public File getDirectory() {
		return this.directory;
	}

	public void setDirectory(File directory) {
		this.directory = directory;
	}

	
	public final File getValidDirectory() {
		File file = this.directory;
		file = (file != null ? file : getWarFileDocumentRoot());
		file = (file != null ? file : getExplodedWarFileDocumentRoot());
		file = (file != null ? file : getCommonDocumentRoot());
		if (file == null && this.logger.isDebugEnabled()) {
			logNoDocumentRoots();
		}
		else if (this.logger.isDebugEnabled()) {
			this.logger.debug("Document root: " + file);
		}
		return file;
	}

	private File getWarFileDocumentRoot() {
		return getArchiveFileDocumentRoot(".war");
	}

	private File getArchiveFileDocumentRoot(String extension) {
		File file = getCodeSourceArchive();
		if (this.logger.isDebugEnabled()) {
			this.logger.debug("Code archive: " + file);
		}
		if (file != null && file.exists() && !file.isDirectory()
				&& file.getName().toLowerCase(Locale.ENGLISH).endsWith(extension)) {
			return file.getAbsoluteFile();
		}
		return null;
	}

	private File getExplodedWarFileDocumentRoot() {
		return getExplodedWarFileDocumentRoot(getCodeSourceArchive());
	}

	private File getCodeSourceArchive() {
		return getCodeSourceArchive(getClass().getProtectionDomain().getCodeSource());
	}

	File getCodeSourceArchive(CodeSource codeSource) {
		try {
			URL location = (codeSource == null ? null : codeSource.getLocation());
			if (location == null) {
				return null;
			}
			String path;
			URLConnection connection = location.openConnection();
			if (connection instanceof JarURLConnection) {
				path = ((JarURLConnection) connection).getJarFile().getName();
			}
			else {
				path = location.toURI().getPath();
			}
			int index = path.indexOf("!/");
			if (index != -1) {
				path = path.substring(0, index);
			}
			return new File(path);
		}
		catch (Exception ex) {
			return null;
		}
	}

	public final File getExplodedWarFileDocumentRoot(File codeSourceFile) {
		if (this.logger.isDebugEnabled()) {
			this.logger.debug("Code archive: " + codeSourceFile);
		}
		if (codeSourceFile != null && codeSourceFile.exists()) {
			String path = codeSourceFile.getAbsolutePath();
			int webInfPathIndex = path
					.indexOf(File.separatorChar + "WEB-INF" + File.separatorChar);
			if (webInfPathIndex >= 0) {
				path = path.substring(0, webInfPathIndex);
				return new File(path);
			}
		}
		return null;
	}

	private File getCommonDocumentRoot() {
		for (String commonDocRoot : COMMON_DOC_ROOTS) {
			File root = new File(commonDocRoot);
			if (root.exists() && root.isDirectory()) {
				return root.getAbsoluteFile();
			}
		}
		return null;
	}

	private void logNoDocumentRoots() {
		this.logger.debug("None of the document roots " + Arrays.asList(COMMON_DOC_ROOTS)
				+ " point to a directory and will be ignored.");
	}

}
