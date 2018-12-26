

package org.springframework.boot.actuate.health;

import java.util.LinkedHashMap;
import java.util.Map;
import java.util.function.Function;

import org.springframework.util.Assert;
import org.springframework.util.ObjectUtils;


public class CompositeReactiveHealthIndicatorFactory {

	private final Function<String, String> healthIndicatorNameFactory;

	public CompositeReactiveHealthIndicatorFactory(
			Function<String, String> healthIndicatorNameFactory) {
		this.healthIndicatorNameFactory = healthIndicatorNameFactory;
	}

	public CompositeReactiveHealthIndicatorFactory() {
		this(new HealthIndicatorNameFactory());
	}

	
	public CompositeReactiveHealthIndicator createReactiveHealthIndicator(
			HealthAggregator healthAggregator,
			Map<String, ReactiveHealthIndicator> reactiveHealthIndicators,
			Map<String, HealthIndicator> healthIndicators) {
		Assert.notNull(healthAggregator, "HealthAggregator must not be null");
		Assert.notNull(reactiveHealthIndicators,
				"ReactiveHealthIndicators must not be null");
		CompositeReactiveHealthIndicator healthIndicator = new CompositeReactiveHealthIndicator(
				healthAggregator);
		merge(reactiveHealthIndicators, healthIndicators)
				.forEach((beanName, indicator) -> {
					String name = this.healthIndicatorNameFactory.apply(beanName);
					healthIndicator.addHealthIndicator(name, indicator);
				});
		return healthIndicator;
	}

	private Map<String, ReactiveHealthIndicator> merge(
			Map<String, ReactiveHealthIndicator> reactiveHealthIndicators,
			Map<String, HealthIndicator> healthIndicators) {
		if (ObjectUtils.isEmpty(healthIndicators)) {
			return reactiveHealthIndicators;
		}
		Map<String, ReactiveHealthIndicator> allIndicators = new LinkedHashMap<>(
				reactiveHealthIndicators);
		healthIndicators.forEach((beanName, indicator) -> {
			String name = this.healthIndicatorNameFactory.apply(beanName);
			allIndicators.computeIfAbsent(name,
					(n) -> new HealthIndicatorReactiveAdapter(indicator));
		});
		return allIndicators;
	}

}
