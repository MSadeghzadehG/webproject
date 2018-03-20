

package org.springframework.boot.context.properties.bind;

import java.lang.annotation.Annotation;
import java.lang.reflect.Array;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.function.Supplier;

import org.springframework.core.ResolvableType;
import org.springframework.core.style.ToStringCreator;
import org.springframework.util.Assert;
import org.springframework.util.ObjectUtils;


public final class Bindable<T> {

	private static final Annotation[] NO_ANNOTATIONS = {};

	private final ResolvableType type;

	private final ResolvableType boxedType;

	private final Supplier<T> value;

	private final Annotation[] annotations;

	private Bindable(ResolvableType type, ResolvableType boxedType, Supplier<T> value,
			Annotation[] annotations) {
		this.type = type;
		this.boxedType = boxedType;
		this.value = value;
		this.annotations = annotations;
	}

	
	public ResolvableType getType() {
		return this.type;
	}

	public ResolvableType getBoxedType() {
		return this.boxedType;
	}

	
	public Supplier<T> getValue() {
		return this.value;
	}

	
	public Annotation[] getAnnotations() {
		return this.annotations;
	}

	
	@SuppressWarnings("unchecked")
	public <A extends Annotation> A getAnnotation(Class<A> type) {
		for (Annotation annotation : this.annotations) {
			if (type.isInstance(annotation)) {
				return (A) annotation;
			}
		}
		return null;
	}

	@Override
	public String toString() {
		ToStringCreator creator = new ToStringCreator(this);
		creator.append("type", this.type);
		creator.append("value", (this.value == null ? "none" : "provided"));
		creator.append("annotations", this.annotations);
		return creator.toString();
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ObjectUtils.nullSafeHashCode(this.type);
		result = prime * result + ObjectUtils.nullSafeHashCode(this.annotations);
		return result;
	}

	@Override
	public boolean equals(Object obj) {
		if (this == obj) {
			return true;
		}
		if (obj == null || getClass() != obj.getClass()) {
			return false;
		}
		Bindable<?> other = (Bindable<?>) obj;
		boolean result = true;
		result = result && nullSafeEquals(this.type.resolve(), other.type.resolve());
		result = result && nullSafeEquals(this.annotations, other.annotations);
		return result;
	}

	private boolean nullSafeEquals(Object o1, Object o2) {
		return ObjectUtils.nullSafeEquals(o1, o2);
	}

	
	public Bindable<T> withAnnotations(Annotation... annotations) {
		return new Bindable<>(this.type, this.boxedType, this.value,
				(annotations == null ? NO_ANNOTATIONS : annotations));
	}

	public Bindable<T> withExistingValue(T existingValue) {
		Assert.isTrue(
				existingValue == null || this.type.isArray()
						|| this.boxedType.resolve().isInstance(existingValue),
				"ExistingValue must be an instance of " + this.type);
		Supplier<T> value = (existingValue == null ? null : () -> existingValue);
		return new Bindable<>(this.type, this.boxedType, value, NO_ANNOTATIONS);
	}

	public Bindable<T> withSuppliedValue(Supplier<T> suppliedValue) {
		return new Bindable<>(this.type, this.boxedType, suppliedValue, NO_ANNOTATIONS);
	}

	
	@SuppressWarnings("unchecked")
	public static <T> Bindable<T> ofInstance(T instance) {
		Assert.notNull(instance, "Instance must not be null");
		Class<T> type = (Class<T>) instance.getClass();
		return of(type).withExistingValue(instance);
	}

	
	public static <T> Bindable<T> of(Class<T> type) {
		Assert.notNull(type, "Type must not be null");
		return of(ResolvableType.forClass(type));
	}

	
	public static <E> Bindable<List<E>> listOf(Class<E> elementType) {
		return of(ResolvableType.forClassWithGenerics(List.class, elementType));
	}

	
	public static <E> Bindable<Set<E>> setOf(Class<E> elementType) {
		return of(ResolvableType.forClassWithGenerics(Set.class, elementType));
	}

	
	public static <K, V> Bindable<Map<K, V>> mapOf(Class<K> keyType, Class<V> valueType) {
		return of(ResolvableType.forClassWithGenerics(Map.class, keyType, valueType));
	}

	
	public static <T> Bindable<T> of(ResolvableType type) {
		Assert.notNull(type, "Type must not be null");
		ResolvableType boxedType = box(type);
		return new Bindable<>(type, boxedType, null, NO_ANNOTATIONS);
	}

	private static ResolvableType box(ResolvableType type) {
		Class<?> resolved = type.resolve();
		if (resolved != null && resolved.isPrimitive()) {
			Object array = Array.newInstance(resolved, 1);
			Class<?> wrapperType = Array.get(array, 0).getClass();
			return ResolvableType.forClass(wrapperType);
		}
		if (resolved != null && resolved.isArray()) {
			return ResolvableType.forArrayComponent(box(type.getComponentType()));
		}
		return type;
	}

}
