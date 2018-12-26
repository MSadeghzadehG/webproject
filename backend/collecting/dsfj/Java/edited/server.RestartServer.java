

package org.springframework.boot.devtools.restart.server;

import java.io.File;
import java.io.IOException;
import java.net.URL;
import java.net.URLClassLoader;
import java.util.Collections;
import java.util.LinkedHashSet;
import java.util.Map.Entry;
import java.util.Set;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import org.springframework.boot.devtools.restart.Restarter;
import org.springframework.boot.devtools.restart.classloader.ClassLoaderFile;
import org.springframework.boot.devtools.restart.classloader.ClassLoaderFile.Kind;
import org.springframework.boot.devtools.restart.classloader.ClassLoaderFiles;
import org.springframework.boot.devtools.restart.classloader.ClassLoaderFiles.SourceFolder;
import org.springframework.util.Assert;
import org.springframework.util.FileCopyUtils;
import org.springframework.util.ResourceUtils;


public class RestartServer {

	private static final Log logger = LogFactory.getLog(RestartServer.class);

	private final SourceFolderUrlFilter sourceFolderUrlFilter;

	private final ClassLoader classLoader;

	
	public RestartServer(SourceFolderUrlFilter sourceFolderUrlFilter) {
		this(sourceFolderUrlFilter, Thread.currentThread().getContextClassLoader());
	}

	
	public RestartServer(SourceFolderUrlFilter sourceFolderUrlFilter,
			ClassLoader classLoader) {
		Assert.notNull(sourceFolderUrlFilter, "SourceFolderUrlFilter must not be null");
		Assert.notNull(classLoader, "ClassLoader must not be null");
		this.sourceFolderUrlFilter = sourceFolderUrlFilter;
		this.classLoader = classLoader;
	}

	
	public void updateAndRestart(ClassLoaderFiles files) {
		Set<URL> urls = new LinkedHashSet<>();
		Set<URL> classLoaderUrls = getClassLoaderUrls();
		for (SourceFolder folder : files.getSourceFolders()) {
			for (Entry<String, ClassLoaderFile> entry : folder.getFilesEntrySet()) {
				for (URL url : classLoaderUrls) {
					if (updateFileSystem(url, entry.getKey(), entry.getValue())) {
						urls.add(url);
					}
				}
			}
			urls.addAll(getMatchingUrls(classLoaderUrls, folder.getName()));
		}
		updateTimeStamp(urls);
		restart(urls, files);
	}

	private boolean updateFileSystem(URL url, String name,
			ClassLoaderFile classLoaderFile) {
		if (!isFolderUrl(url.toString())) {
			return false;
		}
		try {
			File folder = ResourceUtils.getFile(url);
			File file = new File(folder, name);
			if (file.exists() && file.canWrite()) {
				if (classLoaderFile.getKind() == Kind.DELETED) {
					return file.delete();
				}
				FileCopyUtils.copy(classLoaderFile.getContents(), file);
				return true;
			}
		}
		catch (IOException ex) {
					}
		return false;
	}

	private boolean isFolderUrl(String urlString) {
		return urlString.startsWith("file:") && urlString.endsWith("/");
	}

	private Set<URL> getMatchingUrls(Set<URL> urls, String sourceFolder) {
		Set<URL> matchingUrls = new LinkedHashSet<>();
		for (URL url : urls) {
			if (this.sourceFolderUrlFilter.isMatch(sourceFolder, url)) {
				if (logger.isDebugEnabled()) {
					logger.debug("URL " + url + " matched against source folder "
							+ sourceFolder);
				}
				matchingUrls.add(url);
			}
		}
		return matchingUrls;
	}

	private Set<URL> getClassLoaderUrls() {
		Set<URL> urls = new LinkedHashSet<>();
		ClassLoader classLoader = this.classLoader;
		while (classLoader != null) {
			if (classLoader instanceof URLClassLoader) {
				Collections.addAll(urls, ((URLClassLoader) classLoader).getURLs());
			}
			classLoader = classLoader.getParent();
		}
		return urls;

	}

	private void updateTimeStamp(Iterable<URL> urls) {
		for (URL url : urls) {
			updateTimeStamp(url);
		}
	}

	private void updateTimeStamp(URL url) {
		try {
			URL actualUrl = ResourceUtils.extractJarFileURL(url);
			File file = ResourceUtils.getFile(actualUrl, "Jar URL");
			file.setLastModified(System.currentTimeMillis());
		}
		catch (Exception ex) {
					}
	}

	
	protected void restart(Set<URL> urls, ClassLoaderFiles files) {
		Restarter restarter = Restarter.getInstance();
		restarter.addUrls(urls);
		restarter.addClassLoaderFiles(files);
		restarter.restart();
	}

}
