

package org.springframework.boot.actuate.autoconfigure.health;

import java.util.HashSet;
import java.util.Set;

import org.springframework.boot.actuate.health.HealthEndpoint;
import org.springframework.boot.actuate.health.ShowDetails;
import org.springframework.boot.context.properties.ConfigurationProperties;


@ConfigurationProperties("management.endpoint.health")
public class HealthEndpointProperties {

	
	private ShowDetails showDetails = ShowDetails.NEVER;

	
	private Set<String> roles = new HashSet<>();

	public ShowDetails getShowDetails() {
		return this.showDetails;
	}

	public void setShowDetails(ShowDetails showDetails) {
		this.showDetails = showDetails;
	}

	public Set<String> getRoles() {
		return this.roles;
	}

	public void setRoles(Set<String> roles) {
		this.roles = roles;
	}

}
