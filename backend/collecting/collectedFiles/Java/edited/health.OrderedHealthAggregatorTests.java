

package org.springframework.boot.actuate.health;

import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

import org.junit.Before;
import org.junit.Test;

import static org.assertj.core.api.Assertions.assertThat;


public class OrderedHealthAggregatorTests {

	private OrderedHealthAggregator healthAggregator;

	@Before
	public void setup() {
		this.healthAggregator = new OrderedHealthAggregator();
	}

	@Test
	public void defaultOrder() {
		Map<String, Health> healths = new HashMap<>();
		healths.put("h1", new Health.Builder().status(Status.DOWN).build());
		healths.put("h2", new Health.Builder().status(Status.UP).build());
		healths.put("h3", new Health.Builder().status(Status.UNKNOWN).build());
		healths.put("h4", new Health.Builder().status(Status.OUT_OF_SERVICE).build());
		assertThat(this.healthAggregator.aggregate(healths).getStatus())
				.isEqualTo(Status.DOWN);
	}

	@Test
	public void customOrder() {
		this.healthAggregator.setStatusOrder(Status.UNKNOWN, Status.UP,
				Status.OUT_OF_SERVICE, Status.DOWN);
		Map<String, Health> healths = new HashMap<>();
		healths.put("h1", new Health.Builder().status(Status.DOWN).build());
		healths.put("h2", new Health.Builder().status(Status.UP).build());
		healths.put("h3", new Health.Builder().status(Status.UNKNOWN).build());
		healths.put("h4", new Health.Builder().status(Status.OUT_OF_SERVICE).build());
		assertThat(this.healthAggregator.aggregate(healths).getStatus())
				.isEqualTo(Status.UNKNOWN);
	}

	@Test
	public void defaultOrderWithCustomStatus() {
		Map<String, Health> healths = new HashMap<>();
		healths.put("h1", new Health.Builder().status(Status.DOWN).build());
		healths.put("h2", new Health.Builder().status(Status.UP).build());
		healths.put("h3", new Health.Builder().status(Status.UNKNOWN).build());
		healths.put("h4", new Health.Builder().status(Status.OUT_OF_SERVICE).build());
		healths.put("h5", new Health.Builder().status(new Status("CUSTOM")).build());
		assertThat(this.healthAggregator.aggregate(healths).getStatus())
				.isEqualTo(Status.DOWN);
	}

	@Test
	public void customOrderWithCustomStatus() {
		this.healthAggregator.setStatusOrder(
				Arrays.asList("DOWN", "OUT_OF_SERVICE", "UP", "UNKNOWN", "CUSTOM"));
		Map<String, Health> healths = new HashMap<>();
		healths.put("h1", new Health.Builder().status(Status.DOWN).build());
		healths.put("h2", new Health.Builder().status(Status.UP).build());
		healths.put("h3", new Health.Builder().status(Status.UNKNOWN).build());
		healths.put("h4", new Health.Builder().status(Status.OUT_OF_SERVICE).build());
		healths.put("h5", new Health.Builder().status(new Status("CUSTOM")).build());
		assertThat(this.healthAggregator.aggregate(healths).getStatus())
				.isEqualTo(Status.DOWN);
	}

}
