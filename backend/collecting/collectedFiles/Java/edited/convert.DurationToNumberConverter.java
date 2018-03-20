

package org.springframework.boot.convert;

import java.time.Duration;
import java.time.temporal.ChronoUnit;
import java.util.Collections;
import java.util.Set;

import org.springframework.core.convert.TypeDescriptor;
import org.springframework.core.convert.converter.Converter;
import org.springframework.core.convert.converter.GenericConverter;
import org.springframework.util.ReflectionUtils;


final class DurationToNumberConverter implements GenericConverter {

	@Override
	public Set<ConvertiblePair> getConvertibleTypes() {
		return Collections.singleton(new ConvertiblePair(Duration.class, Number.class));
	}

	@Override
	public Object convert(Object source, TypeDescriptor sourceType,
			TypeDescriptor targetType) {
		if (source == null) {
			return null;
		}
		DurationUnit unit = sourceType.getAnnotation(DurationUnit.class);
		return convert((Duration) source, (unit == null ? null : unit.value()),
				targetType.getObjectType());
	}

	private Object convert(Duration source, ChronoUnit unit, Class<?> type) {
		try {
			return type.getConstructor(String.class).newInstance(String
					.valueOf(DurationStyle.Unit.fromChronoUnit(unit).longValue(source)));
		}
		catch (Exception ex) {
			ReflectionUtils.rethrowRuntimeException(ex);
			throw new IllegalStateException(ex);
		}
	}

}
