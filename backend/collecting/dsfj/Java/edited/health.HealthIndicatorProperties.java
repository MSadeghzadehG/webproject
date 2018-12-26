

package org.springframework.boot.actuate.autoconfigure.health;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.springframework.boot.context.properties.ConfigurationProperties;


@ConfigurationProperties(prefix = "management.health.status")
public class HealthIndicatorProperties {

	
	private List<String> order = null;

	
	private final Map<String, Integer> httpMapping = new HashMap<>();

	public List<String> getOrder() {
		return this.order;
	}

	public void setOrder(List<String> statusOrder) {
		if (statusOrder != null && !statusOrder.isEmpty()) {
			this.order = statusOrder;
		}
	}

	public Map<String, Integer> getHttpMapping() {
		return this.httpMapping;
	}

}
