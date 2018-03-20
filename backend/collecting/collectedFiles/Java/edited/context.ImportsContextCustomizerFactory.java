

package org.springframework.boot.test.context;

import java.lang.reflect.Method;
import java.util.List;

import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Import;
import org.springframework.core.annotation.AnnotatedElementUtils;
import org.springframework.test.context.ContextConfigurationAttributes;
import org.springframework.test.context.ContextCustomizer;
import org.springframework.test.context.ContextCustomizerFactory;
import org.springframework.util.Assert;
import org.springframework.util.ReflectionUtils;


class ImportsContextCustomizerFactory implements ContextCustomizerFactory {

	@Override
	public ContextCustomizer createContextCustomizer(Class<?> testClass,
			List<ContextConfigurationAttributes> configAttributes) {
		if (AnnotatedElementUtils.findMergedAnnotation(testClass, Import.class) != null) {
			assertHasNoBeanMethods(testClass);
			return new ImportsContextCustomizer(testClass);
		}
		return null;
	}

	private void assertHasNoBeanMethods(Class<?> testClass) {
		ReflectionUtils.doWithMethods(testClass, this::assertHasNoBeanMethods);
	}

	private void assertHasNoBeanMethods(Method method) {
		Assert.state(!AnnotatedElementUtils.isAnnotated(method, Bean.class),
				"Test classes cannot include @Bean methods");
	}

}
