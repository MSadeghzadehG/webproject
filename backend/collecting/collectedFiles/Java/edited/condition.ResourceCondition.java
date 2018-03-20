

package org.springframework.boot.autoconfigure.condition;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.springframework.boot.autoconfigure.condition.ConditionMessage.Builder;
import org.springframework.boot.autoconfigure.condition.ConditionMessage.Style;
import org.springframework.context.annotation.ConditionContext;
import org.springframework.core.io.Resource;
import org.springframework.core.type.AnnotatedTypeMetadata;


public abstract class ResourceCondition extends SpringBootCondition {

	private final String name;

	private final String property;

	private final String[] resourceLocations;

	
	protected ResourceCondition(String name, String property,
			String... resourceLocations) {
		this.name = name;
		this.property = property;
		this.resourceLocations = resourceLocations;
	}

	@Override
	public ConditionOutcome getMatchOutcome(ConditionContext context,
			AnnotatedTypeMetadata metadata) {
		if (context.getEnvironment().containsProperty(this.property)) {
			return ConditionOutcome.match(
					startConditionMessage().foundExactly("property " + this.property));
		}
		return getResourceOutcome(context, metadata);
	}

	
	protected ConditionOutcome getResourceOutcome(ConditionContext context,
			AnnotatedTypeMetadata metadata) {
		List<String> found = new ArrayList<>();
		for (String location : this.resourceLocations) {
			Resource resource = context.getResourceLoader().getResource(location);
			if (resource != null && resource.exists()) {
				found.add(location);
			}
		}
		if (found.isEmpty()) {
			ConditionMessage message = startConditionMessage()
					.didNotFind("resource", "resources")
					.items(Style.QUOTE, Arrays.asList(this.resourceLocations));
			return ConditionOutcome.noMatch(message);
		}
		ConditionMessage message = startConditionMessage().found("resource", "resources")
				.items(Style.QUOTE, found);
		return ConditionOutcome.match(message);
	}

	protected final Builder startConditionMessage() {
		return ConditionMessage.forCondition("ResourceCondition", "(" + this.name + ")");
	}

}
