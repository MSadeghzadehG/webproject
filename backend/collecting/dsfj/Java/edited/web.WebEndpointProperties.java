

package org.springframework.boot.actuate.autoconfigure.endpoint.web;

import java.util.LinkedHashMap;
import java.util.LinkedHashSet;
import java.util.Map;
import java.util.Set;

import org.springframework.boot.context.properties.ConfigurationProperties;
import org.springframework.util.Assert;
import org.springframework.util.StringUtils;


@ConfigurationProperties(prefix = "management.endpoints.web")
public class WebEndpointProperties {

	private final Exposure exposure = new Exposure();

	
	private String basePath = "/actuator";

	
	private final Map<String, String> pathMapping = new LinkedHashMap<>();

	public Exposure getExposure() {
		return this.exposure;
	}

	public String getBasePath() {
		return this.basePath;
	}

	public void setBasePath(String basePath) {
		Assert.isTrue(basePath.isEmpty() || basePath.startsWith("/"),
				"Base path must start with '/' or be empty");
		this.basePath = cleanBasePath(basePath);
	}

	private String cleanBasePath(String basePath) {
		if (StringUtils.hasText(basePath) && basePath.endsWith("/")) {
			return basePath.substring(0, basePath.length() - 1);
		}
		return basePath;
	}

	public Map<String, String> getPathMapping() {
		return this.pathMapping;
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
