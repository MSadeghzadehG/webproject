

package org.springframework.boot.test.mock.mockito;

import java.util.LinkedHashSet;
import java.util.Set;

import org.springframework.beans.factory.support.BeanDefinitionRegistry;
import org.springframework.context.ConfigurableApplicationContext;
import org.springframework.test.context.ContextCustomizer;
import org.springframework.test.context.MergedContextConfiguration;


class MockitoContextCustomizer implements ContextCustomizer {

	private final Set<Definition> definitions;

	MockitoContextCustomizer(Set<? extends Definition> definitions) {
		this.definitions = new LinkedHashSet<>(definitions);
	}

	@Override
	public void customizeContext(ConfigurableApplicationContext context,
			MergedContextConfiguration mergedContextConfiguration) {
		if (context instanceof BeanDefinitionRegistry) {
			MockitoPostProcessor.register((BeanDefinitionRegistry) context,
					this.definitions);
		}
	}

	@Override
	public int hashCode() {
		return this.definitions.hashCode();
	}

	@Override
	public boolean equals(Object obj) {
		if (obj == this) {
			return true;
		}
		if (obj == null || obj.getClass() != getClass()) {
			return false;
		}
		MockitoContextCustomizer other = (MockitoContextCustomizer) obj;
		return this.definitions.equals(other.definitions);
	}

}
