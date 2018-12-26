

package org.springframework.boot.actuate.autoconfigure.elasticsearch;

import java.time.Duration;
import java.util.ArrayList;
import java.util.List;

import org.springframework.boot.actuate.elasticsearch.ElasticsearchHealthIndicator;
import org.springframework.boot.context.properties.ConfigurationProperties;


@ConfigurationProperties(prefix = "management.health.elasticsearch", ignoreUnknownFields = false)
public class ElasticsearchHealthIndicatorProperties {

	
	private List<String> indices = new ArrayList<>();

	
	private Duration responseTimeout = Duration.ofMillis(100);

	public List<String> getIndices() {
		return this.indices;
	}

	public void setIndices(List<String> indices) {
		this.indices = indices;
	}

	public Duration getResponseTimeout() {
		return this.responseTimeout;
	}

	public void setResponseTimeout(Duration responseTimeout) {
		this.responseTimeout = responseTimeout;
	}

}
