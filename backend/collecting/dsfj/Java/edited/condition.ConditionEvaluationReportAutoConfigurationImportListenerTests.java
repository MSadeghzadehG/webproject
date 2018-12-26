

package org.springframework.boot.autoconfigure.condition;

import java.util.Collections;
import java.util.List;
import java.util.Set;

import org.junit.Before;
import org.junit.Test;

import org.springframework.beans.factory.config.ConfigurableListableBeanFactory;
import org.springframework.beans.factory.support.DefaultListableBeanFactory;
import org.springframework.boot.autoconfigure.AutoConfigurationImportEvent;
import org.springframework.boot.autoconfigure.AutoConfigurationImportListener;
import org.springframework.core.io.support.SpringFactoriesLoader;

import static org.assertj.core.api.Assertions.assertThat;


public class ConditionEvaluationReportAutoConfigurationImportListenerTests {

	private ConditionEvaluationReportAutoConfigurationImportListener listener;

	private final ConfigurableListableBeanFactory beanFactory = new DefaultListableBeanFactory();

	@Before
	public void setup() {
		this.listener = new ConditionEvaluationReportAutoConfigurationImportListener();
		this.listener.setBeanFactory(this.beanFactory);
	}

	@Test
	public void shouldBeInSpringFactories() {
		List<AutoConfigurationImportListener> factories = SpringFactoriesLoader
				.loadFactories(AutoConfigurationImportListener.class, null);
		assertThat(factories).hasAtLeastOneElementOfType(
				ConditionEvaluationReportAutoConfigurationImportListener.class);
	}

	@Test
	public void onAutoConfigurationImportEventShouldRecordCandidates() {
		List<String> candidateConfigurations = Collections.singletonList("Test");
		Set<String> exclusions = Collections.emptySet();
		AutoConfigurationImportEvent event = new AutoConfigurationImportEvent(this,
				candidateConfigurations, exclusions);
		this.listener.onAutoConfigurationImportEvent(event);
		ConditionEvaluationReport report = ConditionEvaluationReport
				.get(this.beanFactory);
		assertThat(report.getUnconditionalClasses())
				.containsExactlyElementsOf(candidateConfigurations);
	}

	@Test
	public void onAutoConfigurationImportEventShouldRecordExclusions() {
		List<String> candidateConfigurations = Collections.emptyList();
		Set<String> exclusions = Collections.singleton("Test");
		AutoConfigurationImportEvent event = new AutoConfigurationImportEvent(this,
				candidateConfigurations, exclusions);
		this.listener.onAutoConfigurationImportEvent(event);
		ConditionEvaluationReport report = ConditionEvaluationReport
				.get(this.beanFactory);
		assertThat(report.getExclusions()).containsExactlyElementsOf(exclusions);
	}

}
