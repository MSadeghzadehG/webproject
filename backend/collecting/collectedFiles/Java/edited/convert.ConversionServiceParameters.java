

package org.springframework.boot.convert;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;
import java.util.function.Consumer;

import org.junit.runners.Parameterized.Parameters;

import org.springframework.core.convert.ConversionService;
import org.springframework.core.convert.converter.ConverterFactory;
import org.springframework.core.convert.converter.GenericConverter;
import org.springframework.format.Formatter;
import org.springframework.format.support.FormattingConversionService;


public class ConversionServiceParameters implements Iterable<Object[]> {

	private final List<Object[]> parameters;

	public ConversionServiceParameters(Formatter<?> formatter) {
		this((Consumer<FormattingConversionService>) (
				conversionService) -> conversionService.addFormatter(formatter));
	}

	public ConversionServiceParameters(ConverterFactory<?, ?> converterFactory) {
		this((Consumer<FormattingConversionService>) (
				conversionService) -> conversionService
						.addConverterFactory(converterFactory));
	}

	public ConversionServiceParameters(GenericConverter converter) {
		this((Consumer<FormattingConversionService>) (
				conversionService) -> conversionService.addConverter(converter));
	}

	public ConversionServiceParameters(
			Consumer<FormattingConversionService> initializer) {
		FormattingConversionService withoutDefaults = new FormattingConversionService();
		initializer.accept(withoutDefaults);
		List<Object[]> parameters = new ArrayList<>();
		parameters.add(
				new Object[] { "without defaults conversion service", withoutDefaults });
		parameters.add(new Object[] { "application conversion service",
				new ApplicationConversionService() });
		this.parameters = Collections.unmodifiableList(parameters);
	}

	@Override
	public Iterator<Object[]> iterator() {
		return this.parameters.iterator();
	}

}
