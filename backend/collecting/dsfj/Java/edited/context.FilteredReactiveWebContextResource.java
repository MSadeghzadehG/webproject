

package org.springframework.boot.web.reactive.context;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;

import org.springframework.core.io.AbstractResource;
import org.springframework.core.io.Resource;
import org.springframework.util.StringUtils;


class FilteredReactiveWebContextResource extends AbstractResource {

	private final String path;

	FilteredReactiveWebContextResource(String path) {
		this.path = path;
	}

	@Override
	public boolean exists() {
		return false;
	}

	@Override
	public Resource createRelative(String relativePath) throws IOException {
		String pathToUse = StringUtils.applyRelativePath(this.path, relativePath);
		return new FilteredReactiveWebContextResource(pathToUse);
	}

	@Override
	public String getDescription() {
		return "ReactiveWebContext resource [" + this.path + "]";
	}

	@Override
	public InputStream getInputStream() throws IOException {
		throw new FileNotFoundException(
				this.getDescription() + " cannot be opened because it does not exist");
	}

}
