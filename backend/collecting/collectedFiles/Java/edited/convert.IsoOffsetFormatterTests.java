

package org.springframework.boot.convert;

import java.time.OffsetDateTime;
import java.time.format.DateTimeFormatter;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;
import org.junit.runners.Parameterized.Parameters;

import org.springframework.core.convert.ConversionService;

import static org.assertj.core.api.Assertions.assertThat;


@RunWith(Parameterized.class)
public class IsoOffsetFormatterTests {

	private final ConversionService conversionService;

	public IsoOffsetFormatterTests(String name, ConversionService conversionService) {
		this.conversionService = conversionService;
	}

	@Test
	public void convertShouldConvertStringToIsoDate() {
		OffsetDateTime now = OffsetDateTime.now();
		OffsetDateTime converted = this.conversionService.convert(
				DateTimeFormatter.ISO_OFFSET_DATE_TIME.format(now), OffsetDateTime.class);
		assertThat(converted).isEqualTo(now);
	}

	@Test
	public void convertShouldConvertIsoDateToString() {
		OffsetDateTime now = OffsetDateTime.now();
		String converted = this.conversionService.convert(now, String.class);
		assertThat(converted).isNotNull()
				.startsWith(now.format(DateTimeFormatter.ISO_OFFSET_DATE_TIME));
	}

	@Parameters(name = "{0}")
	public static Iterable<Object[]> conversionServices() {
		return new ConversionServiceParameters(new IsoOffsetFormatter());
	}

}
