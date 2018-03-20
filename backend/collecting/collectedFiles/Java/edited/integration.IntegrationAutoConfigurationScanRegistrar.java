

package org.springframework.boot.autoconfigure.integration;

import java.util.Collection;
import java.util.Collections;

import org.springframework.beans.BeansException;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.support.BeanDefinitionRegistry;
import org.springframework.boot.autoconfigure.AutoConfigurationPackages;
import org.springframework.core.type.AnnotationMetadata;
import org.springframework.core.type.StandardAnnotationMetadata;
import org.springframework.integration.annotation.IntegrationComponentScan;
import org.springframework.integration.config.IntegrationComponentScanRegistrar;


class IntegrationAutoConfigurationScanRegistrar extends IntegrationComponentScanRegistrar
		implements BeanFactoryAware {

	private BeanFactory beanFactory;

	@Override
	public void setBeanFactory(BeanFactory beanFactory) throws BeansException {
		this.beanFactory = beanFactory;
	}

	@Override
	public void registerBeanDefinitions(AnnotationMetadata importingClassMetadata,
			final BeanDefinitionRegistry registry) {
		super.registerBeanDefinitions(new StandardAnnotationMetadata(
				IntegrationComponentScanConfiguration.class, true), registry);
	}

	@Override
	protected Collection<String> getBasePackages(
			AnnotationMetadata importingClassMetadata, BeanDefinitionRegistry registry) {
		return AutoConfigurationPackages.has(this.beanFactory)
				? AutoConfigurationPackages.get(this.beanFactory)
				: Collections.emptyList();
	}

	@IntegrationComponentScan
	private static class IntegrationComponentScanConfiguration {

	}

}
