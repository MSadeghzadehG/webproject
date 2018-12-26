

package org.springframework.boot.autoconfigure.condition;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;
import java.util.SortedMap;
import java.util.TreeMap;

import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.config.ConfigurableListableBeanFactory;
import org.springframework.context.annotation.Condition;
import org.springframework.context.annotation.ConditionContext;
import org.springframework.core.type.AnnotatedTypeMetadata;
import org.springframework.util.Assert;
import org.springframework.util.ObjectUtils;


public final class ConditionEvaluationReport {

	private static final String BEAN_NAME = "autoConfigurationReport";

	private static final AncestorsMatchedCondition ANCESTOR_CONDITION = new AncestorsMatchedCondition();

	private final SortedMap<String, ConditionAndOutcomes> outcomes = new TreeMap<>();

	private boolean addedAncestorOutcomes;

	private ConditionEvaluationReport parent;

	private List<String> exclusions = Collections.emptyList();

	private Set<String> unconditionalClasses = new HashSet<>();

	
	private ConditionEvaluationReport() {
	}

	
	public void recordConditionEvaluation(String source, Condition condition,
			ConditionOutcome outcome) {
		Assert.notNull(source, "Source must not be null");
		Assert.notNull(condition, "Condition must not be null");
		Assert.notNull(outcome, "Outcome must not be null");
		this.unconditionalClasses.remove(source);
		if (!this.outcomes.containsKey(source)) {
			this.outcomes.put(source, new ConditionAndOutcomes());
		}
		this.outcomes.get(source).add(condition, outcome);
		this.addedAncestorOutcomes = false;
	}

	
	public void recordExclusions(Collection<String> exclusions) {
		Assert.notNull(exclusions, "exclusions must not be null");
		this.exclusions = new ArrayList<>(exclusions);
	}

	
	public void recordEvaluationCandidates(List<String> evaluationCandidates) {
		Assert.notNull(evaluationCandidates, "evaluationCandidates must not be null");
		this.unconditionalClasses = new HashSet<>(evaluationCandidates);
	}

	
	public Map<String, ConditionAndOutcomes> getConditionAndOutcomesBySource() {
		if (!this.addedAncestorOutcomes) {
			for (Map.Entry<String, ConditionAndOutcomes> entry : this.outcomes
					.entrySet()) {
				if (!entry.getValue().isFullMatch()) {
					addNoMatchOutcomeToAncestors(entry.getKey());
				}
			}
			this.addedAncestorOutcomes = true;
		}
		return Collections.unmodifiableMap(this.outcomes);
	}

	private void addNoMatchOutcomeToAncestors(String source) {
		String prefix = source + "$";
		for (Entry<String, ConditionAndOutcomes> entry : this.outcomes.entrySet()) {
			if (entry.getKey().startsWith(prefix)) {
				ConditionOutcome outcome = ConditionOutcome.noMatch(ConditionMessage
						.forCondition("Ancestor " + source).because("did not match"));
				entry.getValue().add(ANCESTOR_CONDITION, outcome);
			}
		}
	}

	
	public List<String> getExclusions() {
		return Collections.unmodifiableList(this.exclusions);
	}

	
	public Set<String> getUnconditionalClasses() {
		return Collections.unmodifiableSet(this.unconditionalClasses);
	}

	
	public ConditionEvaluationReport getParent() {
		return this.parent;
	}

	
	public static ConditionEvaluationReport get(
			ConfigurableListableBeanFactory beanFactory) {
		synchronized (beanFactory) {
			ConditionEvaluationReport report;
			if (beanFactory.containsSingleton(BEAN_NAME)) {
				report = beanFactory.getBean(BEAN_NAME, ConditionEvaluationReport.class);
			}
			else {
				report = new ConditionEvaluationReport();
				beanFactory.registerSingleton(BEAN_NAME, report);
			}
			locateParent(beanFactory.getParentBeanFactory(), report);
			return report;
		}
	}

	private static void locateParent(BeanFactory beanFactory,
			ConditionEvaluationReport report) {
		if (beanFactory != null && report.parent == null
				&& beanFactory.containsBean(BEAN_NAME)) {
			report.parent = beanFactory.getBean(BEAN_NAME,
					ConditionEvaluationReport.class);
		}
	}

	public ConditionEvaluationReport getDelta(ConditionEvaluationReport previousReport) {
		ConditionEvaluationReport delta = new ConditionEvaluationReport();
		for (Entry<String, ConditionAndOutcomes> entry : this.outcomes.entrySet()) {
			ConditionAndOutcomes previous = previousReport.outcomes.get(entry.getKey());
			if (previous == null
					|| previous.isFullMatch() != entry.getValue().isFullMatch()) {
				entry.getValue()
						.forEach((conditionAndOutcome) -> delta.recordConditionEvaluation(
								entry.getKey(), conditionAndOutcome.getCondition(),
								conditionAndOutcome.getOutcome()));
			}
		}
		List<String> newExclusions = new ArrayList<>(this.exclusions);
		newExclusions.removeAll(previousReport.getExclusions());
		delta.recordExclusions(newExclusions);
		List<String> newUnconditionalClasses = new ArrayList<>(this.unconditionalClasses);
		newUnconditionalClasses.removeAll(previousReport.unconditionalClasses);
		delta.unconditionalClasses.addAll(newUnconditionalClasses);
		return delta;
	}

	
	public static class ConditionAndOutcomes implements Iterable<ConditionAndOutcome> {

		private final Set<ConditionAndOutcome> outcomes = new LinkedHashSet<>();

		public void add(Condition condition, ConditionOutcome outcome) {
			this.outcomes.add(new ConditionAndOutcome(condition, outcome));
		}

		
		public boolean isFullMatch() {
			for (ConditionAndOutcome conditionAndOutcomes : this) {
				if (!conditionAndOutcomes.getOutcome().isMatch()) {
					return false;
				}
			}
			return true;
		}

		@Override
		public Iterator<ConditionAndOutcome> iterator() {
			return Collections.unmodifiableSet(this.outcomes).iterator();
		}

	}

	
	public static class ConditionAndOutcome {

		private final Condition condition;

		private final ConditionOutcome outcome;

		public ConditionAndOutcome(Condition condition, ConditionOutcome outcome) {
			this.condition = condition;
			this.outcome = outcome;
		}

		public Condition getCondition() {
			return this.condition;
		}

		public ConditionOutcome getOutcome() {
			return this.outcome;
		}

		@Override
		public boolean equals(Object obj) {
			if (this == obj) {
				return true;
			}
			if (obj == null || getClass() != obj.getClass()) {
				return false;
			}
			ConditionAndOutcome other = (ConditionAndOutcome) obj;
			return (ObjectUtils.nullSafeEquals(this.condition.getClass(),
					other.condition.getClass())
					&& ObjectUtils.nullSafeEquals(this.outcome, other.outcome));
		}

		@Override
		public int hashCode() {
			return this.condition.getClass().hashCode() * 31 + this.outcome.hashCode();
		}

		@Override
		public String toString() {
			return this.condition.getClass() + " " + this.outcome;
		}

	}

	private static class AncestorsMatchedCondition implements Condition {

		@Override
		public boolean matches(ConditionContext context, AnnotatedTypeMetadata metadata) {
			throw new UnsupportedOperationException();
		}

	}

}
