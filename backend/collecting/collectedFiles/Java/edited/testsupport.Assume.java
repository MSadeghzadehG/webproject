

package org.springframework.boot.testsupport;

import org.junit.AssumptionViolatedException;

import org.springframework.util.ClassUtils;


public abstract class Assume {

	public static void javaEight() {
		if (ClassUtils.isPresent("java.security.cert.URICertStoreParameters", null)) {
			throw new AssumptionViolatedException("Assumed Java 8 but got Java 9");
		}
	}

}
