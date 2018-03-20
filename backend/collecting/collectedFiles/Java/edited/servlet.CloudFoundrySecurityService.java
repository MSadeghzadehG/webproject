

package org.springframework.boot.actuate.autoconfigure.cloudfoundry.servlet;

import java.net.URI;
import java.net.URISyntaxException;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.springframework.boot.actuate.autoconfigure.cloudfoundry.AccessLevel;
import org.springframework.boot.actuate.autoconfigure.cloudfoundry.CloudFoundryAuthorizationException;
import org.springframework.boot.actuate.autoconfigure.cloudfoundry.CloudFoundryAuthorizationException.Reason;
import org.springframework.boot.web.client.RestTemplateBuilder;
import org.springframework.http.HttpStatus;
import org.springframework.http.RequestEntity;
import org.springframework.util.Assert;
import org.springframework.web.client.HttpClientErrorException;
import org.springframework.web.client.HttpServerErrorException;
import org.springframework.web.client.HttpStatusCodeException;
import org.springframework.web.client.RestTemplate;


class CloudFoundrySecurityService {

	private final RestTemplate restTemplate;

	private final String cloudControllerUrl;

	private String uaaUrl;

	CloudFoundrySecurityService(RestTemplateBuilder restTemplateBuilder,
			String cloudControllerUrl, boolean skipSslValidation) {
		Assert.notNull(restTemplateBuilder, "RestTemplateBuilder must not be null");
		Assert.notNull(cloudControllerUrl, "CloudControllerUrl must not be null");
		if (skipSslValidation) {
			restTemplateBuilder = restTemplateBuilder
					.requestFactory(SkipSslVerificationHttpRequestFactory.class);
		}
		this.restTemplate = restTemplateBuilder.build();
		this.cloudControllerUrl = cloudControllerUrl;
	}

	
	public AccessLevel getAccessLevel(String token, String applicationId)
			throws CloudFoundryAuthorizationException {
		try {
			URI uri = getPermissionsUri(applicationId);
			RequestEntity<?> request = RequestEntity.get(uri)
					.header("Authorization", "bearer " + token).build();
			Map<?, ?> body = this.restTemplate.exchange(request, Map.class).getBody();
			if (Boolean.TRUE.equals(body.get("read_sensitive_data"))) {
				return AccessLevel.FULL;
			}
			return AccessLevel.RESTRICTED;
		}
		catch (HttpClientErrorException ex) {
			if (ex.getStatusCode().equals(HttpStatus.FORBIDDEN)) {
				throw new CloudFoundryAuthorizationException(Reason.ACCESS_DENIED,
						"Access denied");
			}
			throw new CloudFoundryAuthorizationException(Reason.INVALID_TOKEN,
					"Invalid token", ex);
		}
		catch (HttpServerErrorException ex) {
			throw new CloudFoundryAuthorizationException(Reason.SERVICE_UNAVAILABLE,
					"Cloud controller not reachable");
		}
	}

	private URI getPermissionsUri(String applicationId) {
		try {
			return new URI(this.cloudControllerUrl + "/v2/apps/" + applicationId
					+ "/permissions");
		}
		catch (URISyntaxException ex) {
			throw new IllegalStateException(ex);
		}
	}

	
	public Map<String, String> fetchTokenKeys() {
		try {
			return extractTokenKeys(this.restTemplate
					.getForObject(getUaaUrl() + "/token_keys", Map.class));
		}
		catch (HttpStatusCodeException e) {
			throw new CloudFoundryAuthorizationException(Reason.SERVICE_UNAVAILABLE,
					"UAA not reachable");
		}
	}

	private Map<String, String> extractTokenKeys(Map<?, ?> response) {
		Map<String, String> tokenKeys = new HashMap<>();
		for (Object key : (List<?>) response.get("keys")) {
			Map<?, ?> tokenKey = (Map<?, ?>) key;
			tokenKeys.put((String) tokenKey.get("kid"), (String) tokenKey.get("value"));
		}
		return tokenKeys;
	}

	
	public String getUaaUrl() {
		if (this.uaaUrl == null) {
			try {
				Map<?, ?> response = this.restTemplate
						.getForObject(this.cloudControllerUrl + "/info", Map.class);
				this.uaaUrl = (String) response.get("token_endpoint");
			}
			catch (HttpStatusCodeException ex) {
				throw new CloudFoundryAuthorizationException(Reason.SERVICE_UNAVAILABLE,
						"Unable to fetch token keys from UAA");
			}
		}
		return this.uaaUrl;
	}

}
