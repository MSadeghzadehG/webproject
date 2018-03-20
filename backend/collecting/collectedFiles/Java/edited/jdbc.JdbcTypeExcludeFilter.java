

package org.springframework.boot.test.autoconfigure.jdbc;

import java.util.Collections;
import java.util.Set;

import org.springframework.boot.context.TypeExcludeFilter;
import org.springframework.boot.test.autoconfigure.filter.AnnotationCustomizableTypeExcludeFilter;
import org.springframework.context.annotation.ComponentScan;
import org.springframework.core.annotation.AnnotatedElementUtils;


class JdbcTypeExcludeFilter extends AnnotationCustomizableTypeExcludeFilter {

	private final JdbcTest annotation;

	JdbcTypeExcludeFilter(Class<?> testClass) {
		this.annotation = AnnotatedElementUtils.getMergedAnnotation(testClass,
				JdbcTest.class);
	}

	@Override
	protected boolean hasAnnotation() {
		return this.annotation != null;
	}

	@Override
	protected ComponentScan.Filter[] getFilters(FilterType type) {
		switch (type) {
		case INCLUDE:
			return this.annotation.includeFilters();
		case EXCLUDE:
			return this.annotation.excludeFilters();
		}
		throw new IllegalStateException("Unsupported type " + type);
	}

	@Override
	protected boolean isUseDefaultFilters() {
		return this.annotation.useDefaultFilters();
	}

	@Override
	protected Set<Class<?>> getDefaultIncludes() {
		return Collections.emptySet();
	}

	@Override
	protected Set<Class<?>> getComponentIncludes() {
		return Collections.emptySet();
	}

}
