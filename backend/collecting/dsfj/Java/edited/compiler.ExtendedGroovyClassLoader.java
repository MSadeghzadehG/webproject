

package org.springframework.boot.cli.compiler;

import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.InputStream;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLClassLoader;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import groovy.lang.GroovyClassLoader;
import org.codehaus.groovy.ast.ClassNode;
import org.codehaus.groovy.control.CompilationUnit;
import org.codehaus.groovy.control.CompilerConfiguration;
import org.codehaus.groovy.control.SourceUnit;

import org.springframework.util.Assert;
import org.springframework.util.FileCopyUtils;
import org.springframework.util.StringUtils;


public class ExtendedGroovyClassLoader extends GroovyClassLoader {

	private static final String SHARED_PACKAGE = "org.springframework.boot.groovy";

	private static final URL[] NO_URLS = new URL[] {};

	private final Map<String, byte[]> classResources = new HashMap<>();

	private final GroovyCompilerScope scope;

	private final CompilerConfiguration configuration;

	public ExtendedGroovyClassLoader(GroovyCompilerScope scope) {
		this(scope, createParentClassLoader(scope), new CompilerConfiguration());
	}

	private static ClassLoader createParentClassLoader(GroovyCompilerScope scope) {
		ClassLoader classLoader = Thread.currentThread().getContextClassLoader();
		if (scope == GroovyCompilerScope.DEFAULT) {
			classLoader = new DefaultScopeParentClassLoader(classLoader);
		}
		return classLoader;
	}

	private ExtendedGroovyClassLoader(GroovyCompilerScope scope, ClassLoader parent,
			CompilerConfiguration configuration) {
		super(parent, configuration);
		this.configuration = configuration;
		this.scope = scope;
	}

	@Override
	protected Class<?> findClass(String name) throws ClassNotFoundException {
		try {
			return super.findClass(name);
		}
		catch (ClassNotFoundException ex) {
			if (this.scope == GroovyCompilerScope.DEFAULT
					&& name.startsWith(SHARED_PACKAGE)) {
				Class<?> sharedClass = findSharedClass(name);
				if (sharedClass != null) {
					return sharedClass;
				}
			}
			throw ex;
		}
	}

	private Class<?> findSharedClass(String name) {
		try {
			String path = name.replace('.', '/').concat(".class");
			try (InputStream inputStream = getParent().getResourceAsStream(path)) {
				if (inputStream != null) {
					return defineClass(name, FileCopyUtils.copyToByteArray(inputStream));
				}
			}
			return null;
		}
		catch (Exception ex) {
			return null;
		}
	}

	@Override
	public InputStream getResourceAsStream(String name) {
		InputStream resourceStream = super.getResourceAsStream(name);
		if (resourceStream == null) {
			byte[] bytes = this.classResources.get(name);
			resourceStream = bytes == null ? null : new ByteArrayInputStream(bytes);
		}
		return resourceStream;
	}

	@Override
	public ClassCollector createCollector(CompilationUnit unit, SourceUnit su) {
		InnerLoader loader = AccessController.doPrivileged(getInnerLoader());
		return new ExtendedClassCollector(loader, unit, su);
	}

	private PrivilegedAction<InnerLoader> getInnerLoader() {
		return () -> new InnerLoader(ExtendedGroovyClassLoader.this) {

									@Override
			public URL[] getURLs() {
				return NO_URLS;
			}

		};
	}

	public CompilerConfiguration getConfiguration() {
		return this.configuration;
	}

	
	protected class ExtendedClassCollector extends ClassCollector {

		protected ExtendedClassCollector(InnerLoader loader, CompilationUnit unit,
				SourceUnit su) {
			super(loader, unit, su);
		}

		@Override
		protected Class<?> createClass(byte[] code, ClassNode classNode) {
			Class<?> createdClass = super.createClass(code, classNode);
			ExtendedGroovyClassLoader.this.classResources
					.put(classNode.getName().replace('.', '/') + ".class", code);
			return createdClass;
		}

	}

	
	private static class DefaultScopeParentClassLoader extends ClassLoader {

		private static final String[] GROOVY_JARS_PREFIXES = { "groovy", "antlr", "asm" };

		private final URLClassLoader groovyOnlyClassLoader;

		DefaultScopeParentClassLoader(ClassLoader parent) {
			super(parent);
			this.groovyOnlyClassLoader = new URLClassLoader(getGroovyJars(parent),
					getClass().getClassLoader().getParent());
		}

		private URL[] getGroovyJars(ClassLoader parent) {
			Set<URL> urls = new HashSet<>();
			findGroovyJarsDirectly(parent, urls);
			if (urls.isEmpty()) {
				findGroovyJarsFromClassPath(urls);
			}
			Assert.state(!urls.isEmpty(), "Unable to find groovy JAR");
			return new ArrayList<>(urls).toArray(new URL[0]);
		}

		private void findGroovyJarsDirectly(ClassLoader classLoader, Set<URL> urls) {
			while (classLoader != null) {
				if (classLoader instanceof URLClassLoader) {
					for (URL url : ((URLClassLoader) classLoader).getURLs()) {
						if (isGroovyJar(url.toString())) {
							urls.add(url);
						}
					}
				}
				classLoader = classLoader.getParent();
			}
		}

		private void findGroovyJarsFromClassPath(Set<URL> urls) {
			String classpath = System.getProperty("java.class.path");
			String[] entries = classpath.split(System.getProperty("path.separator"));
			for (String entry : entries) {
				if (isGroovyJar(entry)) {
					File file = new File(entry);
					if (file.canRead()) {
						try {
							urls.add(file.toURI().toURL());
						}
						catch (MalformedURLException ex) {
													}
					}
				}
			}
		}

		private boolean isGroovyJar(String entry) {
			entry = StringUtils.cleanPath(entry);
			for (String jarPrefix : GROOVY_JARS_PREFIXES) {
				if (entry.contains("/" + jarPrefix + "-")) {
					return true;
				}
			}
			return false;
		}

		@Override
		protected Class<?> loadClass(String name, boolean resolve)
				throws ClassNotFoundException {
			if (!name.startsWith("java.")) {
				this.groovyOnlyClassLoader.loadClass(name);
			}
			return super.loadClass(name, resolve);
		}

	}

}
