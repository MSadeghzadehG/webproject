

package org.springframework.boot.context.properties.bind;

import java.lang.annotation.Annotation;
import java.util.Collection;
import java.util.List;
import java.util.TreeSet;
import java.util.function.Supplier;
import java.util.stream.Collectors;

import org.springframework.boot.context.properties.bind.Binder.Context;
import org.springframework.boot.context.properties.source.ConfigurationProperty;
import org.springframework.boot.context.properties.source.ConfigurationPropertyName;
import org.springframework.boot.context.properties.source.ConfigurationPropertyName.Form;
import org.springframework.boot.context.properties.source.ConfigurationPropertySource;
import org.springframework.boot.context.properties.source.IterableConfigurationPropertySource;
import org.springframework.core.ResolvableType;
import org.springframework.util.LinkedMultiValueMap;
import org.springframework.util.MultiValueMap;


abstract class IndexedElementsBinder<T> extends AggregateBinder<T> {

	private static final String INDEX_ZERO = "[0]";

	IndexedElementsBinder(Context context) {
		super(context);
	}

	@Override
	protected boolean isAllowRecursiveBinding(ConfigurationPropertySource source) {
		return source == null || source instanceof IterableConfigurationPropertySource;
	}

	
	protected final void bindIndexed(ConfigurationPropertyName name, Bindable<?> target,
			AggregateElementBinder elementBinder, ResolvableType aggregateType,
			ResolvableType elementType, IndexedCollectionSupplier result) {
		for (ConfigurationPropertySource source : getContext().getSources()) {
			bindIndexed(source, name, target, elementBinder, result, aggregateType,
					elementType);
			if (result.wasSupplied() && result.get() != null) {
				return;
			}
		}
	}

	private void bindIndexed(ConfigurationPropertySource source,
			ConfigurationPropertyName root, Bindable<?> target,
			AggregateElementBinder elementBinder, IndexedCollectionSupplier collection,
			ResolvableType aggregateType, ResolvableType elementType) {
		ConfigurationProperty property = source.getConfigurationProperty(root);
		if (property != null) {
			Object aggregate = convert(property.getValue(), aggregateType,
					target.getAnnotations());
			ResolvableType collectionType = ResolvableType
					.forClassWithGenerics(collection.get().getClass(), elementType);
			Collection<Object> elements = convert(aggregate, collectionType);
			collection.get().addAll(elements);
		}
		else {
			bindIndexed(source, root, elementBinder, collection, elementType);
		}
	}

	private void bindIndexed(ConfigurationPropertySource source,
			ConfigurationPropertyName root, AggregateElementBinder elementBinder,
			IndexedCollectionSupplier collection, ResolvableType elementType) {
		MultiValueMap<String, ConfigurationProperty> knownIndexedChildren = getKnownIndexedChildren(
				source, root);
		for (int i = 0; i < Integer.MAX_VALUE; i++) {
			ConfigurationPropertyName name = root
					.append(i == 0 ? INDEX_ZERO : "[" + i + "]");
			Object value = elementBinder.bind(name, Bindable.of(elementType), source);
			if (value == null) {
				break;
			}
			knownIndexedChildren.remove(name.getLastElement(Form.UNIFORM));
			collection.get().add(value);
		}
		assertNoUnboundChildren(knownIndexedChildren);
	}

	private MultiValueMap<String, ConfigurationProperty> getKnownIndexedChildren(
			ConfigurationPropertySource source, ConfigurationPropertyName root) {
		MultiValueMap<String, ConfigurationProperty> children = new LinkedMultiValueMap<>();
		if (!(source instanceof IterableConfigurationPropertySource)) {
			return children;
		}
		for (ConfigurationPropertyName name : (IterableConfigurationPropertySource) source
				.filter(root::isAncestorOf)) {
			name = name.chop(root.getNumberOfElements() + 1);
			if (name.isLastElementIndexed()) {
				String key = name.getLastElement(Form.UNIFORM);
				ConfigurationProperty value = source.getConfigurationProperty(name);
				children.add(key, value);
			}
		}
		return children;
	}

	private void assertNoUnboundChildren(
			MultiValueMap<String, ConfigurationProperty> children) {
		if (!children.isEmpty()) {
			throw new UnboundConfigurationPropertiesException(
					children.values().stream().flatMap(List::stream)
							.collect(Collectors.toCollection(TreeSet::new)));
		}
	}

	private <C> C convert(Object value, ResolvableType type, Annotation... annotations) {
		value = getContext().getPlaceholdersResolver().resolvePlaceholders(value);
		return getContext().getConverter().convert(value, type, annotations);
	}

	
	protected static class IndexedCollectionSupplier
			extends AggregateSupplier<Collection<Object>> {

		public IndexedCollectionSupplier(Supplier<Collection<Object>> supplier) {
			super(supplier);
		}

	}

}
