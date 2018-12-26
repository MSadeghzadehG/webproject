

package org.springframework.boot.autoconfigure.condition;

import java.util.ArrayList;
import java.util.List;

import org.springframework.context.annotation.Condition;


public abstract class NoneNestedConditions extends AbstractNestedCondition {

	public NoneNestedConditions(ConfigurationPhase configurationPhase) {
		super(configurationPhase);
	}

	@Override
	protected ConditionOutcome getFinalMatchOutcome(MemberMatchOutcomes memberOutcomes) {
		boolean match = memberOutcomes.getMatches().isEmpty();
		List<ConditionMessage> messages = new ArrayList<>();
		messages.add(ConditionMessage.forCondition("NoneNestedConditions")
				.because(memberOutcomes.getMatches().size() + " matched "
						+ memberOutcomes.getNonMatches().size() + " did not"));
		for (ConditionOutcome outcome : memberOutcomes.getAll()) {
			messages.add(outcome.getConditionMessage());
		}
		return new ConditionOutcome(match, ConditionMessage.of(messages));
	}

}
