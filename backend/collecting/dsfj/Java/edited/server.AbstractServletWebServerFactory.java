

package org.springframework.boot.web.servlet.server;

import java.io.File;
import java.io.UnsupportedEncodingException;
import java.net.URL;
import java.net.URLDecoder;
import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Set;

import javax.servlet.ServletContext;
import javax.servlet.ServletException;
import javax.servlet.SessionCookieConfig;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import org.springframework.boot.web.server.AbstractConfigurableWebServerFactory;
import org.springframework.boot.web.server.MimeMappings;
import org.springframework.boot.web.servlet.ServletContextInitializer;
import org.springframework.util.Assert;
import org.springframework.util.ClassUtils;


public abstract class AbstractServletWebServerFactory
		extends AbstractConfigurableWebServerFactory
		implements ConfigurableServletWebServerFactory {

	protected final Log logger = LogFactory.getLog(getClass());

	private String contextPath = "";

	private String displayName;

	private Session session = new Session();

	private boolean registerDefaultServlet = true;

	private MimeMappings mimeMappings = new MimeMappings(MimeMappings.DEFAULT);

	private List<ServletContextInitializer> initializers = new ArrayList<>();

	private Jsp jsp = new Jsp();

	private Map<Locale, Charset> localeCharsetMappings = new HashMap<>();

	private Map<String, String> initParameters = Collections.emptyMap();

	private final DocumentRoot documentRoot = new DocumentRoot(this.logger);

	private final StaticResourceJars staticResourceJars = new StaticResourceJars();

	
	public AbstractServletWebServerFactory() {
	}

	
	public AbstractServletWebServerFactory(int port) {
		super(port);
	}

	
	public AbstractServletWebServerFactory(String contextPath, int port) {
		super(port);
		checkContextPath(contextPath);
		this.contextPath = contextPath;
	}

	
	public String getContextPath() {
		return this.contextPath;
	}

	@Override
	public void setContextPath(String contextPath) {
		checkContextPath(contextPath);
		this.contextPath = contextPath;
	}

	private void checkContextPath(String contextPath) {
		Assert.notNull(contextPath, "ContextPath must not be null");
		if (!contextPath.isEmpty()) {
			if ("/".equals(contextPath)) {
				throw new IllegalArgumentException(
						"Root ContextPath must be specified using an empty string");
			}
			if (!contextPath.startsWith("/") || contextPath.endsWith("/")) {
				throw new IllegalArgumentException(
						"ContextPath must start with '/' and not end with '/'");
			}
		}
	}

	public String getDisplayName() {
		return this.displayName;
	}

	@Override
	public void setDisplayName(String displayName) {
		this.displayName = displayName;
	}

	
	public boolean isRegisterDefaultServlet() {
		return this.registerDefaultServlet;
	}

	@Override
	public void setRegisterDefaultServlet(boolean registerDefaultServlet) {
		this.registerDefaultServlet = registerDefaultServlet;
	}

	
	public MimeMappings getMimeMappings() {
		return this.mimeMappings;
	}

	@Override
	public void setMimeMappings(MimeMappings mimeMappings) {
		this.mimeMappings = new MimeMappings(mimeMappings);
	}

	
	public File getDocumentRoot() {
		return this.documentRoot.getDirectory();
	}

	@Override
	public void setDocumentRoot(File documentRoot) {
		this.documentRoot.setDirectory(documentRoot);
	}

	@Override
	public void setInitializers(List<? extends ServletContextInitializer> initializers) {
		Assert.notNull(initializers, "Initializers must not be null");
		this.initializers = new ArrayList<>(initializers);
	}

	@Override
	public void addInitializers(ServletContextInitializer... initializers) {
		Assert.notNull(initializers, "Initializers must not be null");
		this.initializers.addAll(Arrays.asList(initializers));
	}

	public Jsp getJsp() {
		return this.jsp;
	}

	@Override
	public void setJsp(Jsp jsp) {
		this.jsp = jsp;
	}

	public Session getSession() {
		return this.session;
	}

	@Override
	public void setSession(Session session) {
		this.session = session;
	}

	
	public Map<Locale, Charset> getLocaleCharsetMappings() {
		return this.localeCharsetMappings;
	}

	@Override
	public void setLocaleCharsetMappings(Map<Locale, Charset> localeCharsetMappings) {
		Assert.notNull(localeCharsetMappings, "localeCharsetMappings must not be null");
		this.localeCharsetMappings = localeCharsetMappings;
	}

	@Override
	public void setInitParameters(Map<String, String> initParameters) {
		this.initParameters = initParameters;
	}

	public Map<String, String> getInitParameters() {
		return this.initParameters;
	}

	
	protected final ServletContextInitializer[] mergeInitializers(
			ServletContextInitializer... initializers) {
		List<ServletContextInitializer> mergedInitializers = new ArrayList<>();
		mergedInitializers.add((servletContext) -> this.initParameters
				.forEach(servletContext::setInitParameter));
		mergedInitializers.add(new SessionConfiguringInitializer(this.session));
		mergedInitializers.addAll(Arrays.asList(initializers));
		mergedInitializers.addAll(this.initializers);
		return mergedInitializers.toArray(new ServletContextInitializer[0]);
	}

	
	protected boolean shouldRegisterJspServlet() {
		return this.jsp != null && this.jsp.getRegistered() && ClassUtils
				.isPresent(this.jsp.getClassName(), getClass().getClassLoader());
	}

	
	protected final File getValidDocumentRoot() {
		return this.documentRoot.getValidDirectory();
	}

	protected final List<URL> getUrlsOfJarsWithMetaInfResources() {
		return this.staticResourceJars.getUrls();
	}

	protected final String getDecodedFile(URL url) {
		try {
			return URLDecoder.decode(url.getFile(), "UTF-8");
		}
		catch (UnsupportedEncodingException ex) {
			throw new IllegalStateException(
					"Failed to decode '" + url.getFile() + "' using UTF-8");
		}
	}

	protected final File getValidSessionStoreDir() {
		return getValidSessionStoreDir(true);
	}

	protected final File getValidSessionStoreDir(boolean mkdirs) {
		return this.session.getSessionStoreDirectory().getValidDirectory(mkdirs);
	}

	
	private static class SessionConfiguringInitializer
			implements ServletContextInitializer {

		private final Session session;

		SessionConfiguringInitializer(Session session) {
			this.session = session;
		}

		@Override
		public void onStartup(ServletContext servletContext) throws ServletException {
			if (this.session.getTrackingModes() != null) {
				servletContext
						.setSessionTrackingModes(unwrap(this.session.getTrackingModes()));
			}
			configureSessionCookie(servletContext.getSessionCookieConfig());
		}

		private void configureSessionCookie(SessionCookieConfig config) {
			Session.Cookie cookie = this.session.getCookie();
			if (cookie.getName() != null) {
				config.setName(cookie.getName());
			}
			if (cookie.getDomain() != null) {
				config.setDomain(cookie.getDomain());
			}
			if (cookie.getPath() != null) {
				config.setPath(cookie.getPath());
			}
			if (cookie.getComment() != null) {
				config.setComment(cookie.getComment());
			}
			if (cookie.getHttpOnly() != null) {
				config.setHttpOnly(cookie.getHttpOnly());
			}
			if (cookie.getSecure() != null) {
				config.setSecure(cookie.getSecure());
			}
			if (cookie.getMaxAge() != null) {
				config.setMaxAge((int) cookie.getMaxAge().getSeconds());
			}
		}

		private Set<javax.servlet.SessionTrackingMode> unwrap(
				Set<Session.SessionTrackingMode> modes) {
			if (modes == null) {
				return null;
			}
			Set<javax.servlet.SessionTrackingMode> result = new LinkedHashSet<>();
			for (Session.SessionTrackingMode mode : modes) {
				result.add(javax.servlet.SessionTrackingMode.valueOf(mode.name()));
			}
			return result;
		}

	}

}
