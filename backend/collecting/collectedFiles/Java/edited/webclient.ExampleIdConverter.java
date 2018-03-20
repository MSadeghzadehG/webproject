

package org.springframework.boot.test.autoconfigure.web.reactive.webclient;

import java.util.Collections;
import java.util.Set;
import java.util.UUID;

import org.springframework.boot.test.autoconfigure.web.reactive.WebFluxTest;
import org.springframework.core.convert.TypeDescriptor;
import org.springframework.core.convert.converter.GenericConverter;
import org.springframework.lang.Nullable;
import org.springframework.stereotype.Component;


@Component
public class ExampleIdConverter implements GenericConverter {

	@Override
	public Set<ConvertiblePair> getConvertibleTypes() {
		return Collections.singleton(new ConvertiblePair(String.class, ExampleId.class));
	}

	@Nullable
	@Override
	public Object convert(@Nullable Object source, TypeDescriptor sourceType,
			TypeDescriptor targetType) {
		if (source == null) {
			return null;
		}
		return new ExampleId(UUID.fromString((String) source));
	}

}
