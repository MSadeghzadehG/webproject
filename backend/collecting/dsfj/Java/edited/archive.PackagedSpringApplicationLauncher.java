

package org.springframework.boot.cli.archive;

import java.net.URL;
import java.net.URLClassLoader;
import java.util.Enumeration;
import java.util.jar.Attributes;
import java.util.jar.Manifest;

import org.springframework.boot.cli.app.SpringApplicationLauncher;


public final class PackagedSpringApplicationLauncher {

	
	public static final String SOURCE_ENTRY = "Spring-Application-Source-Classes";

	
	public static final String START_CLASS_ENTRY = "Start-Class";

	private PackagedSpringApplicationLauncher() {
	}

	private void run(String[] args) throws Exception {
		URLClassLoader classLoader = (URLClassLoader) Thread.currentThread()
				.getContextClassLoader();
		new SpringApplicationLauncher(classLoader).launch(getSources(classLoader), args);
	}

	private Class<?>[] getSources(URLClassLoader classLoader) throws Exception {
		Enumeration<URL> urls = classLoader.getResources("META-INF/MANIFEST.MF");
		while (urls.hasMoreElements()) {
			URL url = urls.nextElement();
			Manifest manifest = new Manifest(url.openStream());
			if (isCliPackaged(manifest)) {
				String sources = manifest.getMainAttributes().getValue(SOURCE_ENTRY);
				return loadClasses(classLoader, sources.split(","));
			}
		}
		throw new IllegalStateException(
				"Cannot locate " + SOURCE_ENTRY + " in MANIFEST.MF");
	}

	private boolean isCliPackaged(Manifest manifest) {
		Attributes attributes = manifest.getMainAttributes();
		String startClass = attributes.getValue(START_CLASS_ENTRY);
		return getClass().getName().equals(startClass);
	}

	private Class<?>[] loadClasses(ClassLoader classLoader, String[] names)
			throws ClassNotFoundException {
		Class<?>[] classes = new Class<?>[names.length];
		for (int i = 0; i < names.length; i++) {
			classes[i] = classLoader.loadClass(names[i]);
		}
		return classes;
	}

	public static void main(String[] args) throws Exception {
		new PackagedSpringApplicationLauncher().run(args);
	}

}
