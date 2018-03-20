

package org.springframework.boot.actuate.endpoint.invoke.convert;

import java.time.OffsetDateTime;
import java.time.format.DateTimeFormatter;

import org.springframework.core.convert.converter.Converter;
import org.springframework.core.convert.converter.ConverterRegistry;
import org.springframework.util.StringUtils;


public class IsoOffsetDateTimeConverter implements Converter<String, OffsetDateTime> {

	@Override
	public OffsetDateTime convert(String source) {
		if (StringUtils.hasLength(source)) {
			return OffsetDateTime.parse(source, DateTimeFormatter.ISO_OFFSET_DATE_TIME);
		}
		return null;
	}

	public static void registerConverter(ConverterRegistry registry) {
		registry.addConverter(new IsoOffsetDateTimeConverter());
	}

}
