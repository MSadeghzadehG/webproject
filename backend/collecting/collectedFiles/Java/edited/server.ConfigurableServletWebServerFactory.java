

package org.springframework.boot.web.servlet.server;

import java.io.File;
import java.nio.charset.Charset;
import java.util.List;
import java.util.Locale;
import java.util.Map;

import javax.servlet.ServletContext;

import org.springframework.boot.web.server.ConfigurableWebServerFactory;
import org.springframework.boot.web.server.MimeMappings;
import org.springframework.boot.web.server.WebServerFactoryCustomizer;
import org.springframework.boot.web.servlet.ServletContextInitializer;


public interface ConfigurableServletWebServerFactory
		extends ConfigurableWebServerFactory, ServletWebServerFactory {

	
	void setContextPath(String contextPath);

	
	void setDisplayName(String displayName);

	
	void setSession(Session session);

	
	void setRegisterDefaultServlet(boolean registerDefaultServlet);

	
	void setMimeMappings(MimeMappings mimeMappings);

	
	void setDocumentRoot(File documentRoot);

	
	void setInitializers(List<? extends ServletContextInitializer> initializers);

	
	void addInitializers(ServletContextInitializer... initializers);

	
	void setJsp(Jsp jsp);

	
	void setLocaleCharsetMappings(Map<Locale, Charset> localeCharsetMappings);

	
	void setInitParameters(Map<String, String> initParameters);

}
