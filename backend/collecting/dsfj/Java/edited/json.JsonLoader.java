

package org.springframework.boot.test.json;

import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;

import org.springframework.core.io.ClassPathResource;
import org.springframework.core.io.Resource;
import org.springframework.util.FileCopyUtils;


class JsonLoader {

	private final Class<?> resourceLoadClass;

	private final Charset charset;

	JsonLoader(Class<?> resourceLoadClass, Charset charset) {
		this.resourceLoadClass = resourceLoadClass;
		this.charset = charset == null ? StandardCharsets.UTF_8 : charset;
	}

	Class<?> getResourceLoadClass() {
		return this.resourceLoadClass;
	}

	String getJson(CharSequence source) {
		if (source == null) {
			return null;
		}
		if (source.toString().endsWith(".json")) {
			return getJson(
					new ClassPathResource(source.toString(), this.resourceLoadClass));
		}
		return source.toString();
	}

	String getJson(String path, Class<?> resourceLoadClass) {
		return getJson(new ClassPathResource(path, resourceLoadClass));
	}

	String getJson(byte[] source) {
		return getJson(new ByteArrayInputStream(source));
	}

	String getJson(File source) {
		try {
			return getJson(new FileInputStream(source));
		}
		catch (IOException ex) {
			throw new IllegalStateException("Unable to load JSON from " + source, ex);
		}
	}

	String getJson(Resource source) {
		try {
			return getJson(source.getInputStream());
		}
		catch (IOException ex) {
			throw new IllegalStateException("Unable to load JSON from " + source, ex);
		}
	}

	String getJson(InputStream source) {
		try {
			return FileCopyUtils
					.copyToString(new InputStreamReader(source, this.charset));
		}
		catch (IOException ex) {
			throw new IllegalStateException("Unable to load JSON from InputStream", ex);
		}
	}

}
