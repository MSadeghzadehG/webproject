

package org.springframework.boot.convert;

import java.time.Duration;
import java.time.temporal.ChronoUnit;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;
import org.junit.runners.Parameterized.Parameters;

import org.springframework.core.convert.ConversionService;
import org.springframework.core.convert.TypeDescriptor;

import static org.assertj.core.api.Assertions.assertThat;


@RunWith(Parameterized.class)
public class DurationToNumberConverterTests {

	private final ConversionService conversionService;

	public DurationToNumberConverterTests(String name,
			ConversionService conversionService) {
		this.conversionService = conversionService;
	}

	@Test
	public void convertWithoutStyleShouldReturnMs() {
		Long converted = this.conversionService.convert(Duration.ofSeconds(1),
				Long.class);
		assertThat(converted).isEqualTo(1000);
	}

	@Test
	public void convertWithFormatShouldUseIgnoreFormat() {
		Integer converted = (Integer) this.conversionService.convert(
				Duration.ofSeconds(1),
				MockDurationTypeDescriptor.get(null, DurationStyle.ISO8601),
				TypeDescriptor.valueOf(Integer.class));
		assertThat(converted).isEqualTo(1000);
	}

	@Test
	public void convertWithFormatAndUnitShouldUseFormatAndUnit() {
		Byte converted = (Byte) this.conversionService.convert(Duration.ofSeconds(1),
				MockDurationTypeDescriptor.get(ChronoUnit.SECONDS, null),
				TypeDescriptor.valueOf(Byte.class));
		assertThat(converted).isEqualTo((byte) 1);
	}

	@Parameters(name = "{0}")
	public static Iterable<Object[]> conversionServices() {
		return new ConversionServiceParameters(new DurationToNumberConverter());
	}

}
