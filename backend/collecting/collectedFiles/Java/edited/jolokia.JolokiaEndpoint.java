

package org.springframework.boot.actuate.autoconfigure.jolokia;

import java.util.Map;
import java.util.function.Supplier;

import org.jolokia.http.AgentServlet;

import org.springframework.boot.actuate.endpoint.annotation.Endpoint;
import org.springframework.boot.actuate.endpoint.web.EndpointServlet;
import org.springframework.boot.actuate.endpoint.web.annotation.ServletEndpoint;


@ServletEndpoint(id = "jolokia")
public class JolokiaEndpoint implements Supplier<EndpointServlet> {

	private final Map<String, String> initParameters;

	public JolokiaEndpoint(Map<String, String> initParameters) {
		this.initParameters = initParameters;
	}

	@Override
	public EndpointServlet get() {
		return new EndpointServlet(AgentServlet.class)
				.withInitParameters(this.initParameters);
	}

}
