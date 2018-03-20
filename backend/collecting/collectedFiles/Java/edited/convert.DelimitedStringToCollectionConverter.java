

package org.springframework.boot.convert;

import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.Set;
import java.util.stream.Stream;

import org.springframework.core.CollectionFactory;
import org.springframework.core.convert.ConversionService;
import org.springframework.core.convert.TypeDescriptor;
import org.springframework.core.convert.converter.ConditionalGenericConverter;
import org.springframework.lang.Nullable;
import org.springframework.util.Assert;
import org.springframework.util.StringUtils;


final class DelimitedStringToCollectionConverter implements ConditionalGenericConverter {

	private final ConversionService conversionService;

	DelimitedStringToCollectionConverter(ConversionService conversionService) {
		Assert.notNull(conversionService, "ConversionService must not be null");
		this.conversionService = conversionService;
	}

	@Override
	public Set<ConvertiblePair> getConvertibleTypes() {
		return Collections.singleton(new ConvertiblePair(String.class, Collection.class));
	}

	@Override
	public boolean matches(TypeDescriptor sourceType, TypeDescriptor targetType) {
		return targetType.getElementTypeDescriptor() == null || this.conversionService
				.canConvert(sourceType, targetType.getElementTypeDescriptor());
	}

	@Override
	@Nullable
	public Object convert(@Nullable Object source, TypeDescriptor sourceType,
			TypeDescriptor targetType) {
		if (source == null) {
			return null;
		}
		return convert((String) source, sourceType, targetType);
	}

	private Object convert(String source, TypeDescriptor sourceType,
			TypeDescriptor targetType) {
		Delimiter delimiter = targetType.getAnnotation(Delimiter.class);
		String[] elements = getElements(source,
				(delimiter == null ? "," : delimiter.value()));
		TypeDescriptor elementDescriptor = targetType.getElementTypeDescriptor();
		Collection<Object> target = createCollection(targetType, elementDescriptor,
				elements.length);
		Stream<Object> stream = Arrays.stream(elements).map(String::trim);
		if (elementDescriptor != null) {
			stream = stream.map((element) -> this.conversionService.convert(element,
					sourceType, elementDescriptor));
		}
		stream.forEach(target::add);
		return target;
	}

	private Collection<Object> createCollection(TypeDescriptor targetType,
			TypeDescriptor elementDescriptor, int length) {
		return CollectionFactory.createCollection(targetType.getType(),
				(elementDescriptor != null ? elementDescriptor.getType() : null), length);
	}

	private String[] getElements(String source, String delimiter) {
		return StringUtils.delimitedListToStringArray(source,
				Delimiter.NONE.equals(delimiter) ? null : delimiter);
	}

}
