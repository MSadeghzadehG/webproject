

package org.springframework.boot.test.autoconfigure.web.client;

import java.util.Arrays;
import java.util.Collections;
import java.util.LinkedHashSet;
import java.util.Set;

import org.springframework.boot.context.TypeExcludeFilter;
import org.springframework.boot.jackson.JsonComponent;
import org.springframework.boot.test.autoconfigure.filter.AnnotationCustomizableTypeExcludeFilter;
import org.springframework.context.annotation.ComponentScan.Filter;
import org.springframework.core.annotation.AnnotatedElementUtils;
import org.springframework.util.ClassUtils;


class RestClientExcludeFilter extends AnnotationCustomizableTypeExcludeFilter {

	private static final String DATABIND_MODULE_CLASS_NAME = "com.fasterxml.jackson.databind.Module";

	private static final Set<Class<?>> DEFAULT_INCLUDES;

	static {
		Set<Class<?>> includes = new LinkedHashSet<>();
		if (ClassUtils.isPresent(DATABIND_MODULE_CLASS_NAME,
				RestClientExcludeFilter.class.getClassLoader())) {
			try {
				includes.add(Class.forName(DATABIND_MODULE_CLASS_NAME, true,
						RestClientExcludeFilter.class.getClassLoader()));
			}
			catch (ClassNotFoundException ex) {
				throw new IllegalStateException(
						"Failed to load " + DATABIND_MODULE_CLASS_NAME, ex);
			}
			includes.add(JsonComponent.class);
		}
		DEFAULT_INCLUDES = Collections.unmodifiableSet(includes);
	}

	private final RestClientTest annotation;

	RestClientExcludeFilter(Class<?> testClass) {
		this.annotation = AnnotatedElementUtils.getMergedAnnotation(testClass,
				RestClientTest.class);
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
		}
		throw new IllegalStateException("Unsupported type " + type);
	}

	@Override
	protected boolean isUseDefaultFilters() {
		return this.annotation.useDefaultFilters();
	}

	@Override
	protected Set<Class<?>> getDefaultIncludes() {
		return DEFAULT_INCLUDES;
	}

	@Override
	protected Set<Class<?>> getComponentIncludes() {
		return new LinkedHashSet<>(Arrays.asList(this.annotation.components()));
	}

}
