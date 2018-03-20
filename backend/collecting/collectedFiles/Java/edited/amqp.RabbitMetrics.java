

package org.springframework.boot.actuate.metrics.amqp;

import java.util.Collections;

import com.rabbitmq.client.ConnectionFactory;
import com.rabbitmq.client.impl.MicrometerMetricsCollector;
import io.micrometer.core.instrument.MeterRegistry;
import io.micrometer.core.instrument.Tag;
import io.micrometer.core.instrument.binder.MeterBinder;

import org.springframework.util.Assert;


public class RabbitMetrics implements MeterBinder {

	private final Iterable<Tag> tags;

	private final ConnectionFactory connectionFactory;

	
	public RabbitMetrics(ConnectionFactory connectionFactory, Iterable<Tag> tags) {
		Assert.notNull(connectionFactory, "ConnectionFactory must not be null");
		this.connectionFactory = connectionFactory;
		this.tags = (tags != null ? tags : Collections.emptyList());
	}

	@Override
	public void bindTo(MeterRegistry registry) {
		this.connectionFactory.setMetricsCollector(
				new MicrometerMetricsCollector(registry, "rabbitmq", this.tags));
	}

}
