

package org.springframework.boot.autoconfigure.hazelcast;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Map;

import com.hazelcast.core.HazelcastInstance;
import org.junit.Test;

import org.springframework.beans.factory.support.BeanDefinitionRegistry;
import org.springframework.boot.autoconfigure.AutoConfigurations;
import org.springframework.boot.autoconfigure.data.jpa.EntityManagerFactoryDependsOnPostProcessor;
import org.springframework.boot.autoconfigure.jdbc.DataSourceAutoConfiguration;
import org.springframework.boot.autoconfigure.orm.jpa.HibernateJpaAutoConfiguration;
import org.springframework.boot.test.context.assertj.AssertableApplicationContext;
import org.springframework.boot.test.context.runner.ApplicationContextRunner;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;

import static org.assertj.core.api.Assertions.assertThat;
import static org.mockito.Mockito.mock;


public class HazelcastJpaDependencyAutoConfigurationTests {

	private ApplicationContextRunner contextRunner = new ApplicationContextRunner()
			.withConfiguration(AutoConfigurations.of(DataSourceAutoConfiguration.class,
					HibernateJpaAutoConfiguration.class,
					HazelcastJpaDependencyAutoConfiguration.class))
			.withPropertyValues("spring.datasource.generate-unique-name=true",
					"spring.datasource.initialization-mode=never");

	@Test
	public void registrationIfHazelcastInstanceHasRegularBeanName() {
		this.contextRunner.withUserConfiguration(HazelcastConfiguration.class)
				.run((context) -> {
					assertThat(postProcessors(context))
							.containsKey("hazelcastInstanceJpaDependencyPostProcessor");
					assertThat(entityManagerFactoryDependencies(context))
							.contains("hazelcastInstance");
				});
	}

	@Test
	public void noRegistrationIfHazelcastInstanceHasCustomBeanName() {
		this.contextRunner.withUserConfiguration(HazelcastCustomNameConfiguration.class)
				.run((context) -> {
					assertThat(entityManagerFactoryDependencies(context))
							.doesNotContain("hazelcastInstance");
					assertThat(postProcessors(context)).doesNotContainKey(
							"hazelcastInstanceJpaDependencyPostProcessor");
				});
	}

	@Test
	public void noRegistrationWithNoHazelcastInstance() {
		this.contextRunner.run((context) -> {
			assertThat(entityManagerFactoryDependencies(context))
					.doesNotContain("hazelcastInstance");
			assertThat(postProcessors(context))
					.doesNotContainKey("hazelcastInstanceJpaDependencyPostProcessor");
		});
	}

	@Test
	public void noRegistrationWithNoEntityManagerFactory() {
		new ApplicationContextRunner().withUserConfiguration(HazelcastConfiguration.class)
				.withConfiguration(AutoConfigurations
						.of(HazelcastJpaDependencyAutoConfiguration.class))
				.run((context) -> assertThat(postProcessors(context)).doesNotContainKey(
						"hazelcastInstanceJpaDependencyPostProcessor"));
	}

	private Map<String, EntityManagerFactoryDependsOnPostProcessor> postProcessors(
			AssertableApplicationContext context) {
		return context.getBeansOfType(EntityManagerFactoryDependsOnPostProcessor.class);
	}

	private List<String> entityManagerFactoryDependencies(
			AssertableApplicationContext context) {
		String[] dependsOn = ((BeanDefinitionRegistry) context
				.getSourceApplicationContext()).getBeanDefinition("entityManagerFactory")
						.getDependsOn();
		return dependsOn != null ? Arrays.asList(dependsOn) : Collections.emptyList();
	}

	@Configuration
	static class HazelcastConfiguration {

		@Bean
		public HazelcastInstance hazelcastInstance() {
			return mock(HazelcastInstance.class);
		}

	}

	@Configuration
	static class HazelcastCustomNameConfiguration {

		@Bean
		public HazelcastInstance myHazelcastInstance() {
			return mock(HazelcastInstance.class);
		}

	}

}
