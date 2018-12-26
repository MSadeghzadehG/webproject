

package org.springframework.boot.actuate.web.trace.reactive;

import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import org.springframework.boot.actuate.trace.http.TraceableResponse;
import org.springframework.http.server.reactive.ServerHttpResponse;


class TraceableServerHttpResponse implements TraceableResponse {

	private final ServerHttpResponse response;

	TraceableServerHttpResponse(ServerHttpResponse exchange) {
		this.response = exchange;
	}

	@Override
	public int getStatus() {
		return this.response.getStatusCode() == null ? 200
				: this.response.getStatusCode().value();
	}

	@Override
	public Map<String, List<String>> getHeaders() {
		return new LinkedHashMap<>(this.response.getHeaders());
	}

}
