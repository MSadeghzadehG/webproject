

package org.springframework.boot.context.properties.bind;

import java.util.Collection;
import java.util.List;
import java.util.function.Supplier;

import org.springframework.boot.context.properties.bind.Binder.Context;
import org.springframework.boot.context.properties.source.ConfigurationPropertyName;
import org.springframework.core.CollectionFactory;
import org.springframework.core.ResolvableType;


class CollectionBinder extends IndexedElementsBinder<Collection<Object>> {

	CollectionBinder(Context context) {
		super(context);
	}

	@Override
	protected Object bindAggregate(ConfigurationPropertyName name, Bindable<?> target,
			AggregateElementBinder elementBinder) {
		Class<?> collectionType = (target.getValue() == null
				? target.getType().resolve(Object.class) : List.class);
		ResolvableType aggregateType = ResolvableType.forClassWithGenerics(List.class,
				target.getType().asCollection().getGenerics());
		ResolvableType elementType = target.getType().asCollection().getGeneric();
		IndexedCollectionSupplier result = new IndexedCollectionSupplier(
				() -> CollectionFactory.createCollection(collectionType, 0));
		bindIndexed(name, target, elementBinder, aggregateType, elementType, result);
		if (result.wasSupplied()) {
			return result.get();
		}
		return null;
	}

	@Override
	@SuppressWarnings("unchecked")
	protected Collection<Object> merge(Supplier<?> existing,
			Collection<Object> additional) {
		Collection<Object> existingCollection = (Collection<Object>) existing.get();
		if (existingCollection == null) {
			return additional;
		}
		try {
			existingCollection.clear();
			existingCollection.addAll(additional);
			return existingCollection;
		}
		catch (UnsupportedOperationException ex) {
			return createNewCollection(additional);
		}
	}

	private Collection<Object> createNewCollection(Collection<Object> additional) {
		Collection<Object> merged = CollectionFactory
				.createCollection(additional.getClass(), additional.size());
		merged.addAll(additional);
		return merged;
	}

}
