

package org.springframework.boot.convert;

import java.time.Duration;
import java.util.Collections;
import java.util.Set;

import org.springframework.core.convert.TypeDescriptor;
import org.springframework.core.convert.converter.Converter;
import org.springframework.core.convert.converter.GenericConverter;


final class NumberToDurationConverter implements GenericConverter {

	private final StringToDurationConverter delegate = new StringToDurationConverter();

	@Override
	public Set<ConvertiblePair> getConvertibleTypes() {
		return Collections.singleton(new ConvertiblePair(Number.class, Duration.class));
	}

	@Override
	public Object convert(Object source, TypeDescriptor sourceType,
			TypeDescriptor targetType) {
		return this.delegate.convert(source == null ? null : source.toString(),
				TypeDescriptor.valueOf(String.class), targetType);
	}

}
