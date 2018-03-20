

package org.springframework.boot.convert;

import java.text.ParseException;
import java.time.OffsetDateTime;
import java.time.format.DateTimeFormatter;
import java.util.Locale;

import org.springframework.format.Formatter;


class IsoOffsetFormatter implements Formatter<OffsetDateTime> {

	@Override
	public String print(OffsetDateTime object, Locale locale) {
		return DateTimeFormatter.ISO_OFFSET_DATE_TIME.format(object);
	}

	@Override
	public OffsetDateTime parse(String text, Locale locale) throws ParseException {
		return OffsetDateTime.parse(text, DateTimeFormatter.ISO_OFFSET_DATE_TIME);
	}

}
