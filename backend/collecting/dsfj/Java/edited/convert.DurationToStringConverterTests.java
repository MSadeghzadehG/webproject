

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
public class DurationToStringConverterTests {

	private final ConversionService conversionService;

	public DurationToStringConverterTests(String name,
			ConversionService conversionService) {
		this.conversionService = conversionService;
	}

	@Test
	public void convertWithoutStyleShouldReturnIso8601() {
		String converted = this.conversionService.convert(Duration.ofSeconds(1),
				String.class);
		assertThat(converted).isEqualTo("PT1S");
	}

	@Test
	public void convertWithFormatShouldUseFormatAndMs() {
		String converted = (String) this.conversionService.convert(Duration.ofSeconds(1),
				MockDurationTypeDescriptor.get(null, DurationStyle.SIMPLE),
				TypeDescriptor.valueOf(String.class));
		assertThat(converted).isEqualTo("1000ms");
	}

	@Test
	public void convertWithFormatAndUnitShouldUseFormatAndUnit() {
		String converted = (String) this.conversionService.convert(Duration.ofSeconds(1),
				MockDurationTypeDescriptor.get(ChronoUnit.SECONDS, DurationStyle.SIMPLE),
				TypeDescriptor.valueOf(String.class));
		assertThat(converted).isEqualTo("1s");
	}

	@Parameters(name = "{0}")
	public static Iterable<Object[]> conversionServices() {
		return new ConversionServiceParameters(new DurationToStringConverter());
	}

}
