

package org.springframework.boot.test.autoconfigure.filter;

import java.io.IOException;
import java.lang.annotation.Annotation;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;
import java.util.regex.Pattern;

import org.springframework.beans.BeanUtils;
import org.springframework.context.annotation.ComponentScan.Filter;
import org.springframework.context.annotation.FilterType;
import org.springframework.core.type.classreading.MetadataReader;
import org.springframework.core.type.classreading.MetadataReaderFactory;
import org.springframework.core.type.filter.AnnotationTypeFilter;
import org.springframework.core.type.filter.AspectJTypeFilter;
import org.springframework.core.type.filter.AssignableTypeFilter;
import org.springframework.core.type.filter.RegexPatternTypeFilter;
import org.springframework.core.type.filter.TypeFilter;
import org.springframework.util.Assert;


public class FilterAnnotations implements Iterable<TypeFilter> {

	private final ClassLoader classLoader;

	private final List<TypeFilter> filters;

	public FilterAnnotations(ClassLoader classLoader, Filter[] filters) {
		Assert.notNull(filters, "Filters must not be null");
		this.classLoader = classLoader;
		this.filters = createTypeFilters(filters);
	}

	private List<TypeFilter> createTypeFilters(Filter[] filters) {
		List<TypeFilter> typeFilters = new ArrayList<>();
		for (Filter filter : filters) {
			for (Class<?> filterClass : filter.classes()) {
				typeFilters.add(createTypeFilter(filter.type(), filterClass));
			}
			for (String pattern : filter.pattern()) {
				typeFilters.add(createTypeFilter(filter.type(), pattern));
			}
		}
		return Collections.unmodifiableList(typeFilters);
	}

	@SuppressWarnings("unchecked")
	private TypeFilter createTypeFilter(FilterType filterType, Class<?> filterClass) {
		switch (filterType) {
		case ANNOTATION:
			Assert.isAssignable(Annotation.class, filterClass,
					"An error occurred while processing an ANNOTATION type filter: ");
			return new AnnotationTypeFilter((Class<Annotation>) filterClass);
		case ASSIGNABLE_TYPE:
			return new AssignableTypeFilter(filterClass);
		case CUSTOM:
			Assert.isAssignable(TypeFilter.class, filterClass,
					"An error occurred while processing a CUSTOM type filter: ");
			return BeanUtils.instantiateClass(filterClass, TypeFilter.class);
		}
		throw new IllegalArgumentException(
				"Filter type not supported with Class value: " + filterType);
	}

	private TypeFilter createTypeFilter(FilterType filterType, String pattern) {
		switch (filterType) {
		case ASPECTJ:
			return new AspectJTypeFilter(pattern, this.classLoader);
		case REGEX:
			return new RegexPatternTypeFilter(Pattern.compile(pattern));
		}
		throw new IllegalArgumentException(
				"Filter type not supported with String pattern: " + filterType);
	}

	@Override
	public Iterator<TypeFilter> iterator() {
		return this.filters.iterator();
	}

	public boolean anyMatches(MetadataReader metadataReader,
			MetadataReaderFactory metadataReaderFactory) throws IOException {
		for (TypeFilter filter : this) {
			if (filter.match(metadataReader, metadataReaderFactory)) {
				return true;
			}
		}
		return false;
	}

}
