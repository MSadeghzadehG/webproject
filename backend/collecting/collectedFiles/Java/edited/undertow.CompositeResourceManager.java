

package org.springframework.boot.web.embedded.undertow;

import java.io.IOException;
import java.util.Arrays;
import java.util.List;

import io.undertow.UndertowMessages;
import io.undertow.server.handlers.resource.Resource;
import io.undertow.server.handlers.resource.ResourceChangeListener;
import io.undertow.server.handlers.resource.ResourceManager;


class CompositeResourceManager implements ResourceManager {

	private final List<ResourceManager> resourceManagers;

	CompositeResourceManager(ResourceManager... resourceManagers) {
		this.resourceManagers = Arrays.asList(resourceManagers);
	}

	@Override
	public void close() throws IOException {
		for (ResourceManager resourceManager : this.resourceManagers) {
			resourceManager.close();
		}
	}

	@Override
	public Resource getResource(String path) throws IOException {
		for (ResourceManager resourceManager : this.resourceManagers) {
			Resource resource = resourceManager.getResource(path);
			if (resource != null) {
				return resource;
			}
		}
		return null;
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

}
