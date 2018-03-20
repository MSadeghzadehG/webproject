

package org.springframework.boot;

import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;

import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;

import static org.assertj.core.api.Assertions.assertThat;


public class DefaultApplicationArgumentsTests {

	private static final String[] ARGS = new String[] { "--foo=bar", "--foo=baz",
			"--debug", "spring", "boot" };

	@Rule
	public ExpectedException thrown = ExpectedException.none();

	@Test
	public void argumentsMustNotBeNull() {
		this.thrown.expect(IllegalArgumentException.class);
		this.thrown.expectMessage("Args must not be null");
		new DefaultApplicationArguments(null);
	}

	@Test
	public void getArgs() {
		ApplicationArguments arguments = new DefaultApplicationArguments(ARGS);
		assertThat(arguments.getSourceArgs()).isEqualTo(ARGS);
	}

	@Test
	public void optionNames() {
		ApplicationArguments arguments = new DefaultApplicationArguments(ARGS);
		Set<String> expected = new HashSet<>(Arrays.asList("foo", "debug"));
		assertThat(arguments.getOptionNames()).isEqualTo(expected);
	}

	@Test
	public void containsOption() {
		ApplicationArguments arguments = new DefaultApplicationArguments(ARGS);
		assertThat(arguments.containsOption("foo")).isTrue();
		assertThat(arguments.containsOption("debug")).isTrue();
		assertThat(arguments.containsOption("spring")).isFalse();
	}

	@Test
	public void getOptionValues() {
		ApplicationArguments arguments = new DefaultApplicationArguments(ARGS);
		assertThat(arguments.getOptionValues("foo"))
				.isEqualTo(Arrays.asList("bar", "baz"));
		assertThat(arguments.getOptionValues("debug")).isEmpty();
		assertThat(arguments.getOptionValues("spring")).isNull();
	}

	@Test
	public void getNonOptionArgs() {
		ApplicationArguments arguments = new DefaultApplicationArguments(ARGS);
		assertThat(arguments.getNonOptionArgs()).containsExactly("spring", "boot");
	}

	@Test
	public void getNoNonOptionArgs() {
		ApplicationArguments arguments = new DefaultApplicationArguments(
				new String[] { "--debug" });
		assertThat(arguments.getNonOptionArgs()).isEmpty();
	}

}
