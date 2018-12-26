

package org.springframework.boot.autoconfigure.jndi;

import java.io.IOException;
import java.net.URL;
import java.util.Collections;
import java.util.Enumeration;


public class JndiPropertiesHidingClassLoader extends ClassLoader {

	public JndiPropertiesHidingClassLoader(ClassLoader parent) {
		super(parent);
	}

	@Override
	public Enumeration<URL> getResources(String name) throws IOException {
		if ("jndi.properties".equals(name)) {
			return Collections.enumeration(Collections.emptyList());
		}
		return super.getResources(name);
	}

}
