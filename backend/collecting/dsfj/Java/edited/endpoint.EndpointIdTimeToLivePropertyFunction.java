

package org.springframework.boot.actuate.autoconfigure.endpoint;

import java.time.Duration;
import java.util.function.Function;

import org.springframework.boot.actuate.endpoint.invoker.cache.CachingOperationInvokerAdvisor;
import org.springframework.boot.context.properties.bind.BindResult;
import org.springframework.boot.context.properties.bind.Bindable;
import org.springframework.boot.context.properties.bind.Binder;
import org.springframework.core.env.Environment;
import org.springframework.core.env.PropertyResolver;


class EndpointIdTimeToLivePropertyFunction implements Function<String, Long> {

	private static final Bindable<Duration> DURATION = Bindable.of(Duration.class);

	private final Environment environment;

	
	EndpointIdTimeToLivePropertyFunction(Environment environment) {
		this.environment = environment;
	}

	@Override
	public Long apply(String endpointId) {
		String name = String.format("management.endpoint.%s.cache.time-to-live",
				endpointId);
		BindResult<Duration> duration = Binder.get(this.environment).bind(name, DURATION);
		return duration.map(Duration::toMillis).orElse(null);
	}

}
