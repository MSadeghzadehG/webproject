

package org.springframework.boot.actuate.autoconfigure.trace.http;

import java.util.HashSet;
import java.util.Set;

import org.springframework.boot.actuate.trace.http.Include;
import org.springframework.boot.context.properties.ConfigurationProperties;


@ConfigurationProperties(prefix = "management.trace.http")
public class HttpTraceProperties {

	
	private Set<Include> include = new HashSet<>(Include.defaultIncludes());

	public Set<Include> getInclude() {
		return this.include;
	}

	public void setInclude(Set<Include> include) {
		this.include = include;
	}

}
