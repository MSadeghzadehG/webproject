

package org.springframework.boot;

import java.util.Arrays;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.springframework.core.env.SimpleCommandLinePropertySource;
import org.springframework.util.Assert;


public class DefaultApplicationArguments implements ApplicationArguments {

	private final Source source;

	private final String[] args;

	public DefaultApplicationArguments(String[] args) {
		Assert.notNull(args, "Args must not be null");
		this.source = new Source(args);
		this.args = args;
	}

	@Override
	public String[] getSourceArgs() {
		return this.args;
	}

	@Override
	public Set<String> getOptionNames() {
		String[] names = this.source.getPropertyNames();
		return Collections.unmodifiableSet(new HashSet<>(Arrays.asList(names)));
	}

	@Override
	public boolean containsOption(String name) {
		return this.source.containsProperty(name);
	}

	@Override
	public List<String> getOptionValues(String name) {
		List<String> values = this.source.getOptionValues(name);
		return (values == null ? null : Collections.unmodifiableList(values));
	}

	@Override
	public List<String> getNonOptionArgs() {
		return this.source.getNonOptionArgs();
	}

	private static class Source extends SimpleCommandLinePropertySource {

		Source(String[] args) {
			super(args);
		}

		@Override
		public List<String> getNonOptionArgs() {
			return super.getNonOptionArgs();
		}

		@Override
		public List<String> getOptionValues(String name) {
			return super.getOptionValues(name);
		}

	}

}
