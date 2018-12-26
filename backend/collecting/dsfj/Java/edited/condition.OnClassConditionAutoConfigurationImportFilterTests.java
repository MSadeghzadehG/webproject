

package org.springframework.boot.autoconfigure.condition;

import java.util.Collections;

import org.junit.Before;
import org.junit.Test;

import org.springframework.beans.factory.support.DefaultListableBeanFactory;
import org.springframework.boot.autoconfigure.AutoConfigurationImportFilter;
import org.springframework.boot.autoconfigure.AutoConfigurationMetadata;
import org.springframework.core.io.support.SpringFactoriesLoader;

import static org.assertj.core.api.Assertions.assertThat;
import static org.mockito.BDDMockito.given;
import static org.mockito.Mockito.mock;


public class OnClassConditionAutoConfigurationImportFilterTests {

	private OnClassCondition filter = new OnClassCondition();

	private DefaultListableBeanFactory beanFactory = new DefaultListableBeanFactory();

	@Before
	public void setup() {
		this.filter.setBeanClassLoader(getClass().getClassLoader());
		this.filter.setBeanFactory(this.beanFactory);
	}

	@Test
	public void shouldBeRegistered() {
		assertThat(SpringFactoriesLoader
				.loadFactories(AutoConfigurationImportFilter.class, null))
						.hasAtLeastOneElementOfType(OnClassCondition.class);
	}

	@Test
	public void matchShouldMatchClasses() {
		String[] autoConfigurationClasses = new String[] { "test.match", "test.nomatch" };
		boolean[] result = this.filter.match(autoConfigurationClasses,
				getAutoConfigurationMetadata());
		assertThat(result).containsExactly(true, false);
	}

	@Test
	public void matchShouldRecordOutcome() {
		String[] autoConfigurationClasses = new String[] { "test.match", "test.nomatch" };
		this.filter.match(autoConfigurationClasses, getAutoConfigurationMetadata());
		ConditionEvaluationReport report = ConditionEvaluationReport
				.get(this.beanFactory);
		assertThat(report.getConditionAndOutcomesBySource()).hasSize(1)
				.containsKey("test.nomatch");
	}

	private AutoConfigurationMetadata getAutoConfigurationMetadata() {
		AutoConfigurationMetadata metadata = mock(AutoConfigurationMetadata.class);
		given(metadata.wasProcessed("test.match")).willReturn(true);
		given(metadata.getSet("test.match", "ConditionalOnClass"))
				.willReturn(Collections.singleton("java.io.InputStream"));
		given(metadata.wasProcessed("test.nomatch")).willReturn(true);
		given(metadata.getSet("test.nomatch", "ConditionalOnClass"))
				.willReturn(Collections.singleton("java.io.DoesNotExist"));
		return metadata;
	}

}
