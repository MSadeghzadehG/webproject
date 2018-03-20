

package org.springframework.boot.autoconfigure.security;

import java.util.Arrays;
import java.util.stream.Stream;


public enum StaticResourceLocation {

	
	CSS("/css
	JAVA_SCRIPT("/js
	IMAGES("/images
	WEB_JARS("/webjars
	FAVICON("favicon.ico");

	private final String[] patterns;

	StaticResourceLocation(String... patterns) {
		this.patterns = patterns;
	}

	public Stream<String> getPatterns() {
		return Arrays.stream(this.patterns);
	}

}
