

package org.springframework.boot.actuate.endpoint.web.servlet;

import java.util.Collection;
import java.util.Collections;
import java.util.Map;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.springframework.boot.actuate.endpoint.web.EndpointLinksResolver;
import org.springframework.boot.actuate.endpoint.web.EndpointMapping;
import org.springframework.boot.actuate.endpoint.web.EndpointMediaTypes;
import org.springframework.boot.actuate.endpoint.web.ExposableWebEndpoint;
import org.springframework.boot.actuate.endpoint.web.Link;
import org.springframework.web.bind.annotation.ResponseBody;
import org.springframework.web.cors.CorsConfiguration;
import org.springframework.web.servlet.HandlerMapping;


public class WebMvcEndpointHandlerMapping extends AbstractWebMvcEndpointHandlerMapping {

	private final EndpointLinksResolver linksResolver;

	
	public WebMvcEndpointHandlerMapping(EndpointMapping endpointMapping,
			Collection<ExposableWebEndpoint> endpoints,
			EndpointMediaTypes endpointMediaTypes, CorsConfiguration corsConfiguration,
			EndpointLinksResolver linksResolver) {
		super(endpointMapping, endpoints, endpointMediaTypes, corsConfiguration);
		this.linksResolver = linksResolver;
		setOrder(-100);
	}

	@Override
	@ResponseBody
	protected Map<String, Map<String, Link>> links(HttpServletRequest request,
			HttpServletResponse response) {
		return Collections.singletonMap("_links",
				this.linksResolver.resolveLinks(request.getRequestURL().toString()));
	}

}
