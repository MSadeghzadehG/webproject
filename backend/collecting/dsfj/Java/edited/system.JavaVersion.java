

package org.springframework.boot.system;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import org.springframework.util.ClassUtils;


public enum JavaVersion {

	
	EIGHT("1.8", "java.util.function.Function"),

	
	NINE("1.9", "java.security.cert.URICertStoreParameters");

	private final String name;

	private final boolean available;

	JavaVersion(String name, String className) {
		this.name = name;
		this.available = ClassUtils.isPresent(className, getClass().getClassLoader());
	}

	@Override
	public String toString() {
		return this.name;
	}

	
	public static JavaVersion getJavaVersion() {
		List<JavaVersion> candidates = Arrays.asList(JavaVersion.values());
		Collections.reverse(candidates);
		for (JavaVersion candidate : candidates) {
			if (candidate.available) {
				return candidate;
			}
		}
		return EIGHT;
	}

	
	public boolean isEqualOrNewerThan(JavaVersion version) {
		return compareTo(version) >= 0;
	}

	
	public boolean isOlderThan(JavaVersion version) {
		return compareTo(version) < 0;
	}

}
