

package org.springframework.boot.actuate.autoconfigure.metrics.export.properties;

import java.util.function.Function;
import java.util.function.Supplier;

import org.springframework.util.Assert;


public class PropertiesConfigAdapter<T> {

	private T properties;

	
	public PropertiesConfigAdapter(T properties) {
		Assert.notNull(properties, "Properties must not be null");
		this.properties = properties;
	}

	
	protected final <V> V get(Function<T, V> getter, Supplier<V> fallback) {
		V value = getter.apply(this.properties);
		return (value != null ? value : fallback.get());
	}

}
