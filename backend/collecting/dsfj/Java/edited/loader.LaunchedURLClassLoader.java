

package org.springframework.boot.loader;

import java.io.IOException;
import java.net.JarURLConnection;
import java.net.URL;
import java.net.URLClassLoader;
import java.net.URLConnection;
import java.security.AccessController;
import java.security.PrivilegedExceptionAction;
import java.util.Enumeration;
import java.util.jar.JarFile;

import org.springframework.boot.loader.jar.Handler;


public class LaunchedURLClassLoader extends URLClassLoader {

	static {
		ClassLoader.registerAsParallelCapable();
	}

	
	public LaunchedURLClassLoader(URL[] urls, ClassLoader parent) {
		super(urls, parent);
	}

	@Override
	public URL findResource(String name) {
		Handler.setUseFastConnectionExceptions(true);
		try {
			return super.findResource(name);
		}
		finally {
			Handler.setUseFastConnectionExceptions(false);
		}
	}

	@Override
	public Enumeration<URL> findResources(String name) throws IOException {
		Handler.setUseFastConnectionExceptions(true);
		try {
			return new UseFastConnectionExceptionsEnumeration(super.findResources(name));
		}
		finally {
			Handler.setUseFastConnectionExceptions(false);
		}
	}

	@Override
	protected Class<?> loadClass(String name, boolean resolve)
			throws ClassNotFoundException {
		Handler.setUseFastConnectionExceptions(true);
		try {
			try {
				definePackageIfNecessary(name);
			}
			catch (IllegalArgumentException ex) {
								if (getPackage(name) == null) {
																				throw new AssertionError("Package " + name + " has already been "
							+ "defined but it could not be found");
				}
			}
			return super.loadClass(name, resolve);
		}
		finally {
			Handler.setUseFastConnectionExceptions(false);
		}
	}

	
	private void definePackageIfNecessary(String className) {
		int lastDot = className.lastIndexOf('.');
		if (lastDot >= 0) {
			String packageName = className.substring(0, lastDot);
			if (getPackage(packageName) == null) {
				try {
					definePackage(className, packageName);
				}
				catch (IllegalArgumentException ex) {
										if (getPackage(packageName) == null) {
																								throw new AssertionError(
								"Package " + packageName + " has already been defined "
										+ "but it could not be found");
					}
				}
			}
		}
	}

	private void definePackage(String className, String packageName) {
		try {
			AccessController.doPrivileged((PrivilegedExceptionAction<Object>) () -> {
				String packageEntryName = packageName.replace('.', '/') + "/";
				String classEntryName = className.replace('.', '/') + ".class";
				for (URL url : getURLs()) {
					try {
						URLConnection connection = url.openConnection();
						if (connection instanceof JarURLConnection) {
							JarFile jarFile = ((JarURLConnection) connection)
									.getJarFile();
							if (jarFile.getEntry(classEntryName) != null
									&& jarFile.getEntry(packageEntryName) != null
									&& jarFile.getManifest() != null) {
								definePackage(packageName, jarFile.getManifest(), url);
								return null;
							}
						}
					}
					catch (IOException ex) {
											}
				}
				return null;
			}, AccessController.getContext());
		}
		catch (java.security.PrivilegedActionException ex) {
					}
	}

	
	public void clearCache() {
		for (URL url : getURLs()) {
			try {
				URLConnection connection = url.openConnection();
				if (connection instanceof JarURLConnection) {
					clearCache(connection);
				}
			}
			catch (IOException ex) {
							}
		}

	}

	private void clearCache(URLConnection connection) throws IOException {
		Object jarFile = ((JarURLConnection) connection).getJarFile();
		if (jarFile instanceof org.springframework.boot.loader.jar.JarFile) {
			((org.springframework.boot.loader.jar.JarFile) jarFile).clearCache();
		}
	}

	private static class UseFastConnectionExceptionsEnumeration
			implements Enumeration<URL> {

		private final Enumeration<URL> delegate;

		UseFastConnectionExceptionsEnumeration(Enumeration<URL> delegate) {
			this.delegate = delegate;
		}

		@Override
		public boolean hasMoreElements() {
			Handler.setUseFastConnectionExceptions(true);
			try {
				return this.delegate.hasMoreElements();
			}
			finally {
				Handler.setUseFastConnectionExceptions(false);
			}

		}

		@Override
		public URL nextElement() {
			Handler.setUseFastConnectionExceptions(true);
			try {
				return this.delegate.nextElement();
			}
			finally {
				Handler.setUseFastConnectionExceptions(false);
			}
		}

	}

}
