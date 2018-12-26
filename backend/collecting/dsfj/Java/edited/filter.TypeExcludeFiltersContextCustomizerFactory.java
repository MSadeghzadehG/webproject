

package org.springframework.boot.test.autoconfigure.filter;

import java.util.Arrays;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Set;

import org.springframework.boot.context.TypeExcludeFilter;
import org.springframework.core.annotation.AnnotatedElementUtils;
import org.springframework.test.context.ContextConfigurationAttributes;
import org.springframework.test.context.ContextCustomizer;
import org.springframework.test.context.ContextCustomizerFactory;


class TypeExcludeFiltersContextCustomizerFactory implements ContextCustomizerFactory {

	@Override
	public ContextCustomizer createContextCustomizer(Class<?> testClass,
			List<ContextConfigurationAttributes> configurationAttributes) {
		TypeExcludeFilters annotation = AnnotatedElementUtils
				.findMergedAnnotation(testClass, TypeExcludeFilters.class);
		if (annotation != null) {
			Set<Class<? extends TypeExcludeFilter>> filterClasses = new LinkedHashSet<>(
					Arrays.asList(annotation.value()));
			return new TypeExcludeFiltersContextCustomizer(testClass, filterClasses);
		}
		return null;
	}

}
