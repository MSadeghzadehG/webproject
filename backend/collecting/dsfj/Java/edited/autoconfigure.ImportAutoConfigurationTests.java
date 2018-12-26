

package org.springframework.boot.autoconfigure;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.util.ArrayList;
import java.util.List;

import org.junit.Test;

import org.springframework.context.annotation.AnnotationConfigApplicationContext;
import org.springframework.context.annotation.Configuration;
import org.springframework.util.ClassUtils;

import static org.assertj.core.api.Assertions.assertThat;


public class ImportAutoConfigurationTests {

	@Test
	public void multipleAnnotationsShouldMergeCorrectly() {
		assertThat(getImportedConfigBeans(Config.class)).containsExactly("ConfigA",
				"ConfigB", "ConfigC", "ConfigD");
		assertThat(getImportedConfigBeans(AnotherConfig.class)).containsExactly("ConfigA",
				"ConfigB", "ConfigC", "ConfigD");
	}

	@Test
	public void classesAsAnAlias() {
		assertThat(getImportedConfigBeans(AnotherConfigUsingClasses.class))
				.containsExactly("ConfigA", "ConfigB", "ConfigC", "ConfigD");
	}

	@Test
	public void excluding() {
		assertThat(getImportedConfigBeans(ExcludingConfig.class))
				.containsExactly("ConfigA", "ConfigB", "ConfigD");
	}

	private List<String> getImportedConfigBeans(Class<?> config) {
		AnnotationConfigApplicationContext context = new AnnotationConfigApplicationContext(
				config);
		String shortName = ClassUtils.getShortName(ImportAutoConfigurationTests.class);
		int beginIndex = shortName.length() + 1;
		List<String> orderedConfigBeans = new ArrayList<>();
		for (String bean : context.getBeanDefinitionNames()) {
			if (bean.contains("$Config")) {
				String shortBeanName = ClassUtils.getShortName(bean);
				orderedConfigBeans.add(shortBeanName.substring(beginIndex));
			}
		}
		context.close();
		return orderedConfigBeans;
	}

	@ImportAutoConfiguration({ ConfigD.class, ConfigB.class })
	@MetaImportAutoConfiguration
	static class Config {

	}

	@MetaImportAutoConfiguration
	@ImportAutoConfiguration({ ConfigB.class, ConfigD.class })
	static class AnotherConfig {

	}

	@MetaImportAutoConfiguration
	@ImportAutoConfiguration(classes = { ConfigB.class, ConfigD.class })
	static class AnotherConfigUsingClasses {

	}

	@ImportAutoConfiguration(classes = { ConfigD.class,
			ConfigB.class }, exclude = ConfigC.class)
	@MetaImportAutoConfiguration
	static class ExcludingConfig {

	}

	@Retention(RetentionPolicy.RUNTIME)
	@ImportAutoConfiguration({ ConfigC.class, ConfigA.class })
	@interface MetaImportAutoConfiguration {

	}

	@Configuration
	static class ConfigA {

	}

	@Configuration
	@AutoConfigureAfter(ConfigA.class)
	static class ConfigB {

	}

	@Configuration
	@AutoConfigureAfter(ConfigB.class)
	static class ConfigC {

	}

	@Configuration
	@AutoConfigureAfter(ConfigC.class)
	static class ConfigD {

	}

}
