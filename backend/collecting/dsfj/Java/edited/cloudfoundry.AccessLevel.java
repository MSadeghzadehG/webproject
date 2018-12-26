

package org.springframework.boot.actuate.autoconfigure.cloudfoundry;

import java.util.Arrays;
import java.util.List;


public enum AccessLevel {

	
	RESTRICTED("", "health", "info"),

	
	FULL;

	public static final String REQUEST_ATTRIBUTE = "cloudFoundryAccessLevel";

	private final List<String> endpointIds;

	AccessLevel(String... endpointIds) {
		this.endpointIds = Arrays.asList(endpointIds);
	}

	
	public boolean isAccessAllowed(String endpointId) {
		return this.endpointIds.isEmpty() || this.endpointIds.contains(endpointId);
	}

}
