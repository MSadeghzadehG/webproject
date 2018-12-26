

package org.springframework.boot.web.embedded.jetty;

import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.net.URLConnection;
import java.net.URLStreamHandler;
import java.net.URLStreamHandlerFactory;

import javax.servlet.ServletContainerInitializer;

import org.apache.catalina.webresources.TomcatURLStreamHandlerFactory;
import org.eclipse.jetty.util.component.AbstractLifeCycle;
import org.eclipse.jetty.webapp.WebAppContext;

import org.springframework.util.ClassUtils;


class JasperInitializer extends AbstractLifeCycle {

	private static final String[] INITIALIZER_CLASSES = {
			"org.eclipse.jetty.apache.jsp.JettyJasperInitializer",
			"org.apache.jasper.servlet.JasperInitializer" };

	private final WebAppContext context;

	private final ServletContainerInitializer initializer;

	JasperInitializer(WebAppContext context) {
		this.context = context;
		this.initializer = newInitializer();
	}

	private ServletContainerInitializer newInitializer() {
		for (String className : INITIALIZER_CLASSES) {
			try {
				Class<?> initializerClass = ClassUtils.forName(className, null);
				return (ServletContainerInitializer) initializerClass.newInstance();
			}
			catch (Exception ex) {
							}
		}
		return null;
	}

	@Override
	protected void doStart() throws Exception {
		if (this.initializer == null) {
			return;
		}
		if (ClassUtils.isPresent(
				"org.apache.catalina.webresources.TomcatURLStreamHandlerFactory",
				getClass().getClassLoader())) {
			TomcatURLStreamHandlerFactory.register();
		}
		else {
			try {
				URL.setURLStreamHandlerFactory(new WarUrlStreamHandlerFactory());
			}
			catch (Error ex) {
							}
		}
		ClassLoader classLoader = Thread.currentThread().getContextClassLoader();
		try {
			Thread.currentThread().setContextClassLoader(this.context.getClassLoader());
			try {
				setExtendedListenerTypes(true);
				this.initializer.onStartup(null, this.context.getServletContext());
			}
			finally {
				setExtendedListenerTypes(false);
			}
		}
		finally {
			Thread.currentThread().setContextClassLoader(classLoader);
		}
	}

	private void setExtendedListenerTypes(boolean extended) {
		try {
			this.context.getServletContext().setExtendedListenerTypes(extended);
		}
		catch (NoSuchMethodError ex) {
					}
	}

	
	private static class WarUrlStreamHandlerFactory implements URLStreamHandlerFactory {

		@Override
		public URLStreamHandler createURLStreamHandler(String protocol) {
			if ("war".equals(protocol)) {
				return new WarUrlStreamHandler();
			}
			return null;
		}

	}

	
	private static class WarUrlStreamHandler extends URLStreamHandler {

		@Override
		protected void parseURL(URL u, String spec, int start, int limit) {
			String path = "jar:" + spec.substring("war:".length());
			int separator = path.indexOf("*/");
			if (separator >= 0) {
				path = path.substring(0, separator) + "!/"
						+ path.substring(separator + 2);
			}
			setURL(u, u.getProtocol(), "", -1, null, null, path, null, null);
		}

		@Override
		protected URLConnection openConnection(URL u) throws IOException {
			return new WarURLConnection(u);
		}

	}

	
	private static class WarURLConnection extends URLConnection {

		private final URLConnection connection;

		protected WarURLConnection(URL url) throws IOException {
			super(url);
			this.connection = new URL(url.getFile()).openConnection();
		}

		@Override
		public void connect() throws IOException {
			if (!this.connected) {
				this.connection.connect();
				this.connected = true;
			}
		}

		@Override
		public InputStream getInputStream() throws IOException {
			connect();
			return this.connection.getInputStream();
		}

	}

}
