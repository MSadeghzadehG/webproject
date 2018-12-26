

package org.springframework.boot.actuate.autoconfigure.endpoint.web;

import java.time.Duration;
import java.time.temporal.ChronoUnit;
import java.util.ArrayList;
import java.util.List;

import org.springframework.boot.context.properties.ConfigurationProperties;
import org.springframework.boot.context.properties.PropertyMapper;
import org.springframework.boot.convert.DurationUnit;
import org.springframework.util.CollectionUtils;
import org.springframework.web.cors.CorsConfiguration;


@ConfigurationProperties(prefix = "management.endpoints.web.cors")
public class CorsEndpointProperties {

	
	private List<String> allowedOrigins = new ArrayList<>();

	
	private List<String> allowedMethods = new ArrayList<>();

	
	private List<String> allowedHeaders = new ArrayList<>();

	
	private List<String> exposedHeaders = new ArrayList<>();

	
	private Boolean allowCredentials;

	
	@DurationUnit(ChronoUnit.SECONDS)
	private Duration maxAge = Duration.ofSeconds(1800);

	public List<String> getAllowedOrigins() {
		return this.allowedOrigins;
	}

	public void setAllowedOrigins(List<String> allowedOrigins) {
		this.allowedOrigins = allowedOrigins;
	}

	public List<String> getAllowedMethods() {
		return this.allowedMethods;
	}

	public void setAllowedMethods(List<String> allowedMethods) {
		this.allowedMethods = allowedMethods;
	}

	public List<String> getAllowedHeaders() {
		return this.allowedHeaders;
	}

	public void setAllowedHeaders(List<String> allowedHeaders) {
		this.allowedHeaders = allowedHeaders;
	}

	public List<String> getExposedHeaders() {
		return this.exposedHeaders;
	}

	public void setExposedHeaders(List<String> exposedHeaders) {
		this.exposedHeaders = exposedHeaders;
	}

	public Boolean getAllowCredentials() {
		return this.allowCredentials;
	}

	public void setAllowCredentials(Boolean allowCredentials) {
		this.allowCredentials = allowCredentials;
	}

	public Duration getMaxAge() {
		return this.maxAge;
	}

	public void setMaxAge(Duration maxAge) {
		this.maxAge = maxAge;
	}

	public CorsConfiguration toCorsConfiguration() {
		if (CollectionUtils.isEmpty(this.allowedOrigins)) {
			return null;
		}
		PropertyMapper map = PropertyMapper.get();
		CorsConfiguration configuration = new CorsConfiguration();
		map.from(this::getAllowedOrigins).to(configuration::setAllowedOrigins);
		map.from(this::getAllowedHeaders).whenNot(CollectionUtils::isEmpty)
				.to(configuration::setAllowedHeaders);
		map.from(this::getAllowedMethods).whenNot(CollectionUtils::isEmpty)
				.to(configuration::setAllowedMethods);
		map.from(this::getExposedHeaders).whenNot(CollectionUtils::isEmpty)
				.to(configuration::setExposedHeaders);
		map.from(this::getMaxAge).whenNonNull().as(Duration::getSeconds)
				.to(configuration::setMaxAge);
		map.from(this::getAllowCredentials).whenNonNull()
				.to(configuration::setAllowCredentials);
		return configuration;
	}

}
