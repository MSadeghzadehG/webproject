

package org.springframework.boot.autoconfigure.web.format;

import java.time.format.DateTimeFormatter;
import java.time.format.ResolverStyle;

import org.joda.time.format.DateTimeFormatterBuilder;

import org.springframework.format.datetime.DateFormatter;
import org.springframework.format.datetime.DateFormatterRegistrar;
import org.springframework.format.datetime.joda.JodaTimeFormatterRegistrar;
import org.springframework.format.datetime.standard.DateTimeFormatterRegistrar;
import org.springframework.format.number.NumberFormatAnnotationFormatterFactory;
import org.springframework.format.number.money.CurrencyUnitFormatter;
import org.springframework.format.number.money.Jsr354NumberFormatAnnotationFormatterFactory;
import org.springframework.format.number.money.MonetaryAmountFormatter;
import org.springframework.format.support.DefaultFormattingConversionService;
import org.springframework.util.ClassUtils;
import org.springframework.util.StringUtils;


public class WebConversionService extends DefaultFormattingConversionService {

	private static final boolean JSR_354_PRESENT = ClassUtils.isPresent(
			"javax.money.MonetaryAmount", WebConversionService.class.getClassLoader());

	private static final boolean JODA_TIME_PRESENT = ClassUtils.isPresent(
			"org.joda.time.LocalDate", WebConversionService.class.getClassLoader());

	private final String dateFormat;

	
	public WebConversionService(String dateFormat) {
		super(false);
		this.dateFormat = (StringUtils.hasText(dateFormat) ? dateFormat : null);
		if (this.dateFormat != null) {
			addFormatters();
		}
		else {
			addDefaultFormatters(this);
		}
	}

	private void addFormatters() {
		addFormatterForFieldAnnotation(new NumberFormatAnnotationFormatterFactory());
		if (JSR_354_PRESENT) {
			addFormatter(new CurrencyUnitFormatter());
			addFormatter(new MonetaryAmountFormatter());
			addFormatterForFieldAnnotation(
					new Jsr354NumberFormatAnnotationFormatterFactory());
		}
		registerJsr310();
		if (JODA_TIME_PRESENT) {
			registerJodaTime();
		}
		registerJavaDate();
	}

	private void registerJsr310() {
		DateTimeFormatterRegistrar dateTime = new DateTimeFormatterRegistrar();
		if (this.dateFormat != null) {
			dateTime.setDateFormatter(DateTimeFormatter.ofPattern(this.dateFormat)
					.withResolverStyle(ResolverStyle.STRICT));
		}
		dateTime.registerFormatters(this);
	}

	private void registerJodaTime() {
		JodaTimeFormatterRegistrar jodaTime = new JodaTimeFormatterRegistrar();
		if (this.dateFormat != null) {
			jodaTime.setDateFormatter(new DateTimeFormatterBuilder()
					.appendPattern(this.dateFormat).toFormatter());
		}
		jodaTime.registerFormatters(this);
	}

	private void registerJavaDate() {
		DateFormatterRegistrar dateFormatterRegistrar = new DateFormatterRegistrar();
		if (this.dateFormat != null) {
			DateFormatter dateFormatter = new DateFormatter(this.dateFormat);
			dateFormatterRegistrar.setFormatter(dateFormatter);
		}
		dateFormatterRegistrar.registerFormatters(this);
	}

}
