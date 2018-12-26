

package org.springframework.boot.actuate.endpoint.web.reactive;

import java.util.Collection;
import java.util.Collections;
import java.util.Map;

import org.springframework.beans.factory.InitializingBean;
import org.springframework.boot.actuate.endpoint.web.EndpointLinksResolver;
import org.springframework.boot.actuate.endpoint.web.EndpointMapping;
import org.springframework.boot.actuate.endpoint.web.EndpointMediaTypes;
import org.springframework.boot.actuate.endpoint.web.ExposableWebEndpoint;
import org.springframework.boot.actuate.endpoint.web.Link;
import org.springframework.web.bind.annotation.ResponseBody;
import org.springframework.web.cors.CorsConfiguration;
import org.springframework.web.reactive.HandlerMapping;
import org.springframework.web.server.ServerWebExchange;
import org.springframework.web.util.UriComponentsBuilder;


public class WebFluxEndpointHandlerMapping extends AbstractWebFluxEndpointHandlerMapping
		implements InitializingBean {

	private final EndpointLinksResolver linksResolver;

	
	public WebFluxEndpointHandlerMapping(EndpointMapping endpointMapping,
			Collection<ExposableWebEndpoint> endpoints,
			EndpointMediaTypes endpointMediaTypes, CorsConfiguration corsConfiguration,
			EndpointLinksResolver linksResolver) {
		super(endpointMapping, endpoints, endpointMediaTypes, corsConfiguration);
		this.linksResolver = linksResolver;
		setOrder(-100);
	}

	@Override
	@ResponseBody
	protected Map<String, Map<String, Link>> links(ServerWebExchange exchange) {
		String requestUri = UriComponentsBuilder.fromUri(exchange.getRequest().getURI())
				.replaceQuery(null).toUriString();
		return Collections.singletonMap("_links",
				this.linksResolver.resolveLinks(requestUri));
	}

}
