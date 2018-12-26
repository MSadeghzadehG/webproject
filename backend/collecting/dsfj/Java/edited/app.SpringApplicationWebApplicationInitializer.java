

package org.springframework.boot.cli.app;

import java.io.IOException;
import java.io.InputStream;
import java.util.jar.Manifest;

import javax.servlet.ServletContext;
import javax.servlet.ServletException;

import org.springframework.boot.builder.SpringApplicationBuilder;
import org.springframework.boot.web.servlet.support.SpringBootServletInitializer;


public class SpringApplicationWebApplicationInitializer
		extends SpringBootServletInitializer {

	
	public static final String SOURCE_ENTRY = "Spring-Application-Source-Classes";

	private String[] sources;

	@Override
	public void onStartup(ServletContext servletContext) throws ServletException {
		try {
			this.sources = getSources(servletContext);
		}
		catch (IOException ex) {
			throw new IllegalStateException(ex);
		}
		super.onStartup(servletContext);
	}

	private String[] getSources(ServletContext servletContext) throws IOException {
		Manifest manifest = getManifest(servletContext);
		if (manifest == null) {
			throw new IllegalStateException("Unable to read manifest");
		}
		String sources = manifest.getMainAttributes().getValue(SOURCE_ENTRY);
		return sources.split(",");
	}

	private Manifest getManifest(ServletContext servletContext) throws IOException {
		InputStream stream = servletContext.getResourceAsStream("/META-INF/MANIFEST.MF");
		return (stream == null ? null : new Manifest(stream));
	}

	@Override
	protected SpringApplicationBuilder configure(SpringApplicationBuilder builder) {
		try {
			ClassLoader classLoader = Thread.currentThread().getContextClassLoader();
			Class<?>[] sourceClasses = new Class<?>[this.sources.length];
			for (int i = 0; i < this.sources.length; i++) {
				sourceClasses[i] = classLoader.loadClass(this.sources[i]);
			}
			return builder.sources(sourceClasses)
					.properties("spring.groovy.template.check-template-location=false");
		}
		catch (Exception ex) {
			throw new IllegalStateException(ex);
		}
	}

}
