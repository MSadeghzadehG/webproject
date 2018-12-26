

package org.springframework.boot.convert;

import java.time.Duration;
import java.time.temporal.ChronoUnit;
import java.util.Collections;
import java.util.Set;

import org.springframework.core.convert.TypeDescriptor;
import org.springframework.core.convert.converter.Converter;
import org.springframework.core.convert.converter.GenericConverter;


final class DurationToStringConverter implements GenericConverter {

	@Override
	public Set<ConvertiblePair> getConvertibleTypes() {
		return Collections.singleton(new ConvertiblePair(Duration.class, String.class));
	}

	@Override
	public Object convert(Object source, TypeDescriptor sourceType,
			TypeDescriptor targetType) {
		if (source == null) {
			return null;
		}
		DurationFormat format = sourceType.getAnnotation(DurationFormat.class);
		DurationUnit unit = sourceType.getAnnotation(DurationUnit.class);
		return convert((Duration) source, (format == null ? null : format.value()),
				(unit == null ? null : unit.value()));
	}

	private String convert(Duration source, DurationStyle style, ChronoUnit unit) {
		style = (style != null ? style : DurationStyle.ISO8601);
		return style.print(source, unit);
	}

}
