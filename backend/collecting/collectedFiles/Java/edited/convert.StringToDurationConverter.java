

package org.springframework.boot.convert;

import java.time.Duration;
import java.time.temporal.ChronoUnit;
import java.util.Collections;
import java.util.Set;

import org.springframework.core.convert.TypeDescriptor;
import org.springframework.core.convert.converter.Converter;
import org.springframework.core.convert.converter.GenericConverter;
import org.springframework.util.ObjectUtils;


final class StringToDurationConverter implements GenericConverter {

	@Override
	public Set<ConvertiblePair> getConvertibleTypes() {
		return Collections.singleton(new ConvertiblePair(String.class, Duration.class));
	}

	@Override
	public Object convert(Object source, TypeDescriptor sourceType,
			TypeDescriptor targetType) {
		if (ObjectUtils.isEmpty(source)) {
			return null;
		}
		DurationFormat format = targetType.getAnnotation(DurationFormat.class);
		DurationUnit unit = targetType.getAnnotation(DurationUnit.class);
		return convert(source.toString(), (format == null ? null : format.value()),
				(unit == null ? null : unit.value()));
	}

	private Duration convert(String source, DurationStyle style, ChronoUnit unit) {
		style = (style != null ? style : DurationStyle.detect(source));
		return style.parse(source, unit);
	}

}
