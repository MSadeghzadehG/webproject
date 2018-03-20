

package org.springframework.boot.autoconfigure.web.format;

import java.util.Date;

import org.joda.time.DateTime;
import org.joda.time.LocalDate;
import org.junit.Test;

import static org.assertj.core.api.Assertions.assertThat;


public class WebConversionServiceTests {

	@Test
	public void customDateFormat() {
		WebConversionService conversionService = new WebConversionService("dd*MM*yyyy");
		Date date = new DateTime(2018, 1, 1, 20, 30).toDate();
		assertThat(conversionService.convert(date, String.class)).isEqualTo("01*01*2018");
		LocalDate jodaDate = LocalDate.fromDateFields(date);
		assertThat(conversionService.convert(jodaDate, String.class))
				.isEqualTo("01*01*2018");
		java.time.LocalDate localDate = java.time.LocalDate.of(2018, 1, 1);
		assertThat(conversionService.convert(localDate, String.class))
				.isEqualTo("01*01*2018");
	}

}
