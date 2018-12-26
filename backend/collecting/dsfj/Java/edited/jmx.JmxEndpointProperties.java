

package org.springframework.boot.actuate.autoconfigure.endpoint.jmx;

import java.util.LinkedHashSet;
import java.util.Properties;
import java.util.Set;

import org.springframework.boot.context.properties.ConfigurationProperties;
import org.springframework.core.env.Environment;
import org.springframework.util.StringUtils;


@ConfigurationProperties("management.endpoints.jmx")
public class JmxEndpointProperties {

	private final Exposure exposure = new Exposure();

	
	private String domain = "org.springframework.boot";

	
	private boolean uniqueNames = false;

	
	private final Properties staticNames = new Properties();

	public JmxEndpointProperties(Environment environment) {
		String defaultDomain = environment.getProperty("spring.jmx.default-domain");
		if (StringUtils.hasText(defaultDomain)) {
			this.domain = defaultDomain;
		}
	}

	public Exposure getExposure() {
		return this.exposure;
	}

	public String getDomain() {
		return this.domain;
	}

	public void setDomain(String domain) {
		this.domain = domain;
	}

	public boolean isUniqueNames() {
		return this.uniqueNames;
	}

	public void setUniqueNames(boolean uniqueNames) {
		this.uniqueNames = uniqueNames;
	}

	public Properties getStaticNames() {
		return this.staticNames;
	}

	public static class Exposure {

		
		private Set<String> include = new LinkedHashSet<>();

		
		private Set<String> exclude = new LinkedHashSet<>();

		public Set<String> getInclude() {
			return this.include;
		}

		public void setInclude(Set<String> include) {
			this.include = include;
		}

		public Set<String> getExclude() {
			return this.exclude;
		}

		public void setExclude(Set<String> exclude) {
			this.exclude = exclude;
		}

	}

}
