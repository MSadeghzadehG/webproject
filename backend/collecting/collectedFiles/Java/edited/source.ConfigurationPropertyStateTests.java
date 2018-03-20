

package org.springframework.boot.context.properties.source;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;

import static org.assertj.core.api.Assertions.assertThat;


public class ConfigurationPropertyStateTests {

	@Rule
	public ExpectedException thrown = ExpectedException.none();

	@Test
	public void searchWhenIterableIsNullShouldThrowException() {
		this.thrown.expect(IllegalArgumentException.class);
		this.thrown.expectMessage("Source must not be null");
		ConfigurationPropertyState.search(null, (e) -> true);
	}

	@Test
	public void searchWhenPredicateIsNullShouldThrowException() {
		this.thrown.expect(IllegalArgumentException.class);
		this.thrown.expectMessage("Predicate must not be null");
		ConfigurationPropertyState.search(Collections.emptyList(), null);
	}

	@Test
	public void searchWhenContainsItemShouldReturnPresent() {
		List<String> source = Arrays.asList("a", "b", "c");
		ConfigurationPropertyState result = ConfigurationPropertyState.search(source,
				"b"::equals);
		assertThat(result).isEqualTo(ConfigurationPropertyState.PRESENT);
	}

	@Test
	public void searchWhenContainsNoItemShouldReturnAbsent() {
		List<String> source = Arrays.asList("a", "x", "c");
		ConfigurationPropertyState result = ConfigurationPropertyState.search(source,
				"b"::equals);
		assertThat(result).isEqualTo(ConfigurationPropertyState.ABSENT);
	}

}
