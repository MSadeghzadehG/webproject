

package org.springframework.boot.actuate.metrics.web.client;

import java.io.IOException;
import java.net.URI;
import java.util.Map;
import java.util.concurrent.TimeUnit;

import io.micrometer.core.instrument.MeterRegistry;
import io.micrometer.core.instrument.Timer;

import org.springframework.core.NamedThreadLocal;
import org.springframework.http.HttpRequest;
import org.springframework.http.client.ClientHttpRequestExecution;
import org.springframework.http.client.ClientHttpRequestInterceptor;
import org.springframework.http.client.ClientHttpResponse;
import org.springframework.web.util.UriTemplateHandler;


class MetricsClientHttpRequestInterceptor implements ClientHttpRequestInterceptor {

	private static final ThreadLocal<String> urlTemplate = new NamedThreadLocal<>(
			"Rest Template URL Template");

	private final MeterRegistry meterRegistry;

	private final RestTemplateExchangeTagsProvider tagProvider;

	private final String metricName;

	MetricsClientHttpRequestInterceptor(MeterRegistry meterRegistry,
			RestTemplateExchangeTagsProvider tagProvider, String metricName) {
		this.tagProvider = tagProvider;
		this.meterRegistry = meterRegistry;
		this.metricName = metricName;
	}

	@Override
	public ClientHttpResponse intercept(HttpRequest request, byte[] body,
			ClientHttpRequestExecution execution) throws IOException {
		long startTime = System.nanoTime();
		ClientHttpResponse response = null;
		try {
			response = execution.execute(request, body);
			return response;
		}
		finally {
			getTimeBuilder(request, response).register(this.meterRegistry)
					.record(System.nanoTime() - startTime, TimeUnit.NANOSECONDS);
			urlTemplate.remove();
		}
	}

	UriTemplateHandler createUriTemplateHandler(UriTemplateHandler delegate) {
		return new UriTemplateHandler() {

			@Override
			public URI expand(String url, Map<String, ?> arguments) {
				urlTemplate.set(url);
				return delegate.expand(url, arguments);
			}

			@Override
			public URI expand(String url, Object... arguments) {
				urlTemplate.set(url);
				return delegate.expand(url, arguments);
			}

		};
	}

	private Timer.Builder getTimeBuilder(HttpRequest request,
			ClientHttpResponse response) {
		return Timer.builder(this.metricName)
				.tags(this.tagProvider.getTags(urlTemplate.get(), request, response))
				.description("Timer of RestTemplate operation");
	}

}
