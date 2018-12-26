

package org.springframework.boot.devtools.classpath;

import java.io.File;
import java.net.URL;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import org.springframework.util.ResourceUtils;


public class ClassPathFolders implements Iterable<File> {

	private static final Log logger = LogFactory.getLog(ClassPathFolders.class);

	private final List<File> folders = new ArrayList<>();

	public ClassPathFolders(URL[] urls) {
		if (urls != null) {
			addUrls(urls);
		}
	}

	private void addUrls(URL[] urls) {
		for (URL url : urls) {
			addUrl(url);
		}
	}

	private void addUrl(URL url) {
		if (url.getProtocol().equals("file") && url.getPath().endsWith("/")) {
			try {
				this.folders.add(ResourceUtils.getFile(url));
			}
			catch (Exception ex) {
				logger.warn("Unable to get classpath URL " + url);
				logger.trace("Unable to get classpath URL " + url, ex);
			}
		}
	}

	@Override
	public Iterator<File> iterator() {
		return Collections.unmodifiableList(this.folders).iterator();
	}

}
