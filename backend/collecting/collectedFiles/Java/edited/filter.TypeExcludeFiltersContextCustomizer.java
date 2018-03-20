

package org.springframework.boot.test.autoconfigure.filter;

import java.io.IOException;
import java.lang.reflect.Constructor;
import java.util.Collections;
import java.util.LinkedHashSet;
import java.util.Set;

import org.springframework.boot.context.TypeExcludeFilter;
import org.springframework.context.ConfigurableApplicationContext;
import org.springframework.core.type.classreading.MetadataReader;
import org.springframework.core.type.classreading.MetadataReaderFactory;
import org.springframework.test.context.ContextCustomizer;
import org.springframework.test.context.MergedContextConfiguration;
import org.springframework.util.ReflectionUtils;


class TypeExcludeFiltersContextCustomizer implements ContextCustomizer {

	private static final String EXCLUDE_FILTER_BEAN_NAME = TypeExcludeFilters.class
			.getName();

	private final Set<TypeExcludeFilter> filters;

	TypeExcludeFiltersContextCustomizer(Class<?> testClass,
			Set<Class<? extends TypeExcludeFilter>> filterClasses) {
		this.filters = instantiateTypeExcludeFilters(testClass, filterClasses);
	}

	private Set<TypeExcludeFilter> instantiateTypeExcludeFilters(Class<?> testClass,
			Set<Class<? extends TypeExcludeFilter>> filterClasses) {
		Set<TypeExcludeFilter> filters = new LinkedHashSet<>();
		for (Class<? extends TypeExcludeFilter> filterClass : filterClasses) {
			filters.add(instantiateTypeExcludeFilter(testClass, filterClass));
		}
		return Collections.unmodifiableSet(filters);
	}

	private TypeExcludeFilter instantiateTypeExcludeFilter(Class<?> testClass,
			Class<?> filterClass) {
		try {
			Constructor<?> constructor = getTypeExcludeFilterConstructor(filterClass);
			ReflectionUtils.makeAccessible(constructor);
			if (constructor.getParameterCount() == 1) {
				return (TypeExcludeFilter) constructor.newInstance(testClass);
			}
			return (TypeExcludeFilter) constructor.newInstance();
		}
		catch (Exception ex) {
			throw new IllegalStateException("Unable to create filter for " + filterClass,
					ex);
		}
	}

	@Override
	public int hashCode() {
		return this.filters.hashCode();
	}

	@Override
	public boolean equals(Object obj) {
		return (obj != null && getClass() == obj.getClass() && this.filters
				.equals(((TypeExcludeFiltersContextCustomizer) obj).filters));
	}

	@Override
	public void customizeContext(ConfigurableApplicationContext context,
			MergedContextConfiguration mergedContextConfiguration) {
		if (!this.filters.isEmpty()) {
			context.getBeanFactory().registerSingleton(EXCLUDE_FILTER_BEAN_NAME,
					createDelegatingTypeExcludeFilter());
		}
	}

	private TypeExcludeFilter createDelegatingTypeExcludeFilter() {
		return new TypeExcludeFilter() {

			@Override
			public boolean match(MetadataReader metadataReader,
					MetadataReaderFactory metadataReaderFactory) throws IOException {
				for (TypeExcludeFilter filter : TypeExcludeFiltersContextCustomizer.this.filters) {
					if (filter.match(metadataReader, metadataReaderFactory)) {
						return true;
					}
				}
				return false;
			}

		};
	}

	private Constructor<?> getTypeExcludeFilterConstructor(Class<?> type)
			throws NoSuchMethodException {
		try {
			return type.getDeclaredConstructor(Class.class);
		}
		catch (Exception ex) {
			return type.getDeclaredConstructor();
		}
	}

}
