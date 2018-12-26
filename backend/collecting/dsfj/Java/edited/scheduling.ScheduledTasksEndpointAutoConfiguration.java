

package org.springframework.boot.actuate.autoconfigure.scheduling;

import java.util.Collections;
import java.util.List;

import org.springframework.beans.factory.ObjectProvider;
import org.springframework.boot.actuate.autoconfigure.endpoint.condition.ConditionalOnEnabledEndpoint;
import org.springframework.boot.actuate.scheduling.ScheduledTasksEndpoint;
import org.springframework.boot.autoconfigure.EnableAutoConfiguration;
import org.springframework.boot.autoconfigure.condition.ConditionalOnMissingBean;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.scheduling.config.ScheduledTaskHolder;


@Configuration
public class ScheduledTasksEndpointAutoConfiguration {

	@Bean
	@ConditionalOnMissingBean
	@ConditionalOnEnabledEndpoint
	public ScheduledTasksEndpoint scheduledTasksEndpoint(
			ObjectProvider<List<ScheduledTaskHolder>> holders) {
		return new ScheduledTasksEndpoint(holders.getIfAvailable(Collections::emptyList));
	}

}
