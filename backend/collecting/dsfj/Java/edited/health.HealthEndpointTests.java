

package org.springframework.boot.actuate.health;

import java.util.HashMap;
import java.util.Map;

import org.junit.Test;

import static org.assertj.core.api.Assertions.assertThat;
import static org.assertj.core.api.Assertions.entry;


public class HealthEndpointTests {

	@Test
	public void statusAndFullDetailsAreExposed() {
		Map<String, HealthIndicator> healthIndicators = new HashMap<>();
		healthIndicators.put("up", () -> new Health.Builder().status(Status.UP)
				.withDetail("first", "1").build());
		healthIndicators.put("upAgain", () -> new Health.Builder().status(Status.UP)
				.withDetail("second", "2").build());
		HealthEndpoint endpoint = new HealthEndpoint(
				createHealthIndicator(healthIndicators));
		Health health = endpoint.health();
		assertThat(health.getStatus()).isEqualTo(Status.UP);
		assertThat(health.getDetails()).containsOnlyKeys("up", "upAgain");
		Health upHealth = (Health) health.getDetails().get("up");
		assertThat(upHealth.getDetails()).containsOnly(entry("first", "1"));
		Health upAgainHealth = (Health) health.getDetails().get("upAgain");
		assertThat(upAgainHealth.getDetails()).containsOnly(entry("second", "2"));
	}

	private HealthIndicator createHealthIndicator(
			Map<String, HealthIndicator> healthIndicators) {
		return new CompositeHealthIndicatorFactory()
				.createHealthIndicator(new OrderedHealthAggregator(), healthIndicators);
	}

}
