

package org.springframework.boot.test.autoconfigure.data.mongo;

import java.util.Collections;
import java.util.Set;

import org.springframework.boot.context.TypeExcludeFilter;
import org.springframework.boot.test.autoconfigure.filter.AnnotationCustomizableTypeExcludeFilter;
import org.springframework.context.annotation.ComponentScan.Filter;
import org.springframework.core.annotation.AnnotatedElementUtils;


class DataMongoTypeExcludeFilter extends AnnotationCustomizableTypeExcludeFilter {

	private final DataMongoTest annotation;

	DataMongoTypeExcludeFilter(Class<?> testClass) {
		this.annotation = AnnotatedElementUtils.getMergedAnnotation(testClass,
				DataMongoTest.class);
	}

	@Override
	protected boolean hasAnnotation() {
		return this.annotation != null;
	}

	@Override
	protected Filter[] getFilters(FilterType type) {
		switch (type) {
		case INCLUDE:
			return this.annotation.includeFilters();
		case EXCLUDE:
			return this.annotation.excludeFilters();
		default:
			throw new IllegalStateException("Unsupported type " + type);
		}
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
