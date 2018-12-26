

package org.springframework.boot.actuate.health;

import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

import org.springframework.boot.actuate.endpoint.web.WebEndpointResponse;
import org.springframework.util.Assert;


public class HealthStatusHttpMapper {

	private Map<String, Integer> statusMapping = new HashMap<>();

	
	public HealthStatusHttpMapper() {
		setupDefaultStatusMapping();
	}

	private void setupDefaultStatusMapping() {
		addStatusMapping(Status.DOWN, WebEndpointResponse.STATUS_SERVICE_UNAVAILABLE);
		addStatusMapping(Status.OUT_OF_SERVICE,
				WebEndpointResponse.STATUS_SERVICE_UNAVAILABLE);
	}

	
	public void setStatusMapping(Map<String, Integer> statusMapping) {
		Assert.notNull(statusMapping, "StatusMapping must not be null");
		this.statusMapping = new HashMap<>(statusMapping);
	}

	
	public void addStatusMapping(Map<String, Integer> statusMapping) {
		Assert.notNull(statusMapping, "StatusMapping must not be null");
		this.statusMapping.putAll(statusMapping);
	}

	
	public void addStatusMapping(Status status, Integer httpStatus) {
		Assert.notNull(status, "Status must not be null");
		Assert.notNull(httpStatus, "HttpStatus must not be null");
		addStatusMapping(status.getCode(), httpStatus);
	}

	
	public void addStatusMapping(String statusCode, Integer httpStatus) {
		Assert.notNull(statusCode, "StatusCode must not be null");
		Assert.notNull(httpStatus, "HttpStatus must not be null");
		this.statusMapping.put(statusCode, httpStatus);
	}

	
	public Map<String, Integer> getStatusMapping() {
		return Collections.unmodifiableMap(this.statusMapping);
	}

	
	public int mapStatus(Status status) {
		String code = getUniformValue(status.getCode());
		if (code != null) {
			return this.statusMapping.keySet().stream()
					.filter((key) -> code.equals(getUniformValue(key)))
					.map(this.statusMapping::get).findFirst()
					.orElse(WebEndpointResponse.STATUS_OK);
		}
		return WebEndpointResponse.STATUS_OK;
	}

	private String getUniformValue(String code) {
		if (code == null) {
			return null;
		}
		StringBuilder builder = new StringBuilder();
		for (char ch : code.toCharArray()) {
			if (Character.isAlphabetic(ch) || Character.isDigit(ch)) {
				builder.append(Character.toLowerCase(ch));
			}
		}
		return builder.toString();
	}

}
