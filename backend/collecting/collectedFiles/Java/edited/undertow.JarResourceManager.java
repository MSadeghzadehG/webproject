

package org.springframework.boot.web.embedded.undertow;

import java.io.File;
import java.io.IOException;
import java.net.URL;

import io.undertow.UndertowMessages;
import io.undertow.server.handlers.resource.Resource;
import io.undertow.server.handlers.resource.ResourceChangeListener;
import io.undertow.server.handlers.resource.ResourceManager;
import io.undertow.server.handlers.resource.URLResource;


class JarResourceManager implements ResourceManager {

	private final String jarPath;

	JarResourceManager(File jarFile) {
		this(jarFile.getAbsolutePath());
	}

	JarResourceManager(String jarPath) {
		this.jarPath = jarPath;
	}

	@Override
	public Resource getResource(String path) throws IOException {
		URL url = new URL("jar:file:" + this.jarPath + "!"
				+ (path.startsWith("/") ? path : "/" + path));
		URLResource resource = new URLResource(url, path);
		if (resource.getContentLength() < 0) {
			return null;
		}
		return resource;
	}

	@Override
	public boolean isResourceChangeListenerSupported() {
		return false;
	}

	@Override
	public void registerResourceChangeListener(ResourceChangeListener listener) {
		throw UndertowMessages.MESSAGES.resourceChangeListenerNotSupported();

	}

	@Override
	public void removeResourceChangeListener(ResourceChangeListener listener) {
		throw UndertowMessages.MESSAGES.resourceChangeListenerNotSupported();
	}

	@Override
	public void close() throws IOException {

	}

}
