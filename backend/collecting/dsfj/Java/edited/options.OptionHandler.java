

package org.springframework.boot.cli.command.options;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.Comparator;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.TreeSet;

import joptsimple.BuiltinHelpFormatter;
import joptsimple.HelpFormatter;
import joptsimple.OptionDescriptor;
import joptsimple.OptionParser;
import joptsimple.OptionSet;
import joptsimple.OptionSpecBuilder;

import org.springframework.boot.cli.command.OptionParsingCommand;
import org.springframework.boot.cli.command.status.ExitStatus;


public class OptionHandler {

	private OptionParser parser;

	private String help;

	private Collection<OptionHelp> optionHelp;

	public OptionSpecBuilder option(String name, String description) {
		return getParser().accepts(name, description);
	}

	public OptionSpecBuilder option(Collection<String> aliases, String description) {
		return getParser().acceptsAll(aliases, description);
	}

	public OptionParser getParser() {
		if (this.parser == null) {
			this.parser = new OptionParser();
			options();
		}
		return this.parser;
	}

	protected void options() {
	}

	public final ExitStatus run(String... args) throws Exception {
		String[] argsToUse = args.clone();
		for (int i = 0; i < argsToUse.length; i++) {
			if ("-cp".equals(argsToUse[i])) {
				argsToUse[i] = "--cp";
			}
		}
		OptionSet options = getParser().parse(argsToUse);
		return run(options);
	}

	
	protected ExitStatus run(OptionSet options) throws Exception {
		return ExitStatus.OK;
	}

	public String getHelp() {
		if (this.help == null) {
			getParser().formatHelpWith(new BuiltinHelpFormatter(80, 2));
			OutputStream out = new ByteArrayOutputStream();
			try {
				getParser().printHelpOn(out);
			}
			catch (IOException ex) {
				return "Help not available";
			}
			this.help = out.toString().replace(" --cp ", " -cp  ");
		}
		return this.help;
	}

	public Collection<OptionHelp> getOptionsHelp() {
		if (this.optionHelp == null) {
			OptionHelpFormatter formatter = new OptionHelpFormatter();
			getParser().formatHelpWith(formatter);
			try {
				getParser().printHelpOn(new ByteArrayOutputStream());
			}
			catch (Exception ex) {
							}
			this.optionHelp = formatter.getOptionHelp();
		}
		return this.optionHelp;
	}

	private static class OptionHelpFormatter implements HelpFormatter {

		private final List<OptionHelp> help = new ArrayList<>();

		@Override
		public String format(Map<String, ? extends OptionDescriptor> options) {
			Comparator<OptionDescriptor> comparator = Comparator.comparing(
					(optionDescriptor) -> optionDescriptor.options().iterator().next());
			Set<OptionDescriptor> sorted = new TreeSet<>(comparator);
			sorted.addAll(options.values());
			for (OptionDescriptor descriptor : sorted) {
				if (!descriptor.representsNonOptions()) {
					this.help.add(new OptionHelpAdapter(descriptor));
				}
			}
			return "";
		}

		public Collection<OptionHelp> getOptionHelp() {
			return Collections.unmodifiableList(this.help);
		}

	}

	private static class OptionHelpAdapter implements OptionHelp {

		private final Set<String> options;

		private final String description;

		OptionHelpAdapter(OptionDescriptor descriptor) {
			this.options = new LinkedHashSet<>();
			for (String option : descriptor.options()) {
				this.options.add((option.length() == 1 ? "-" : "--") + option);
			}
			if (this.options.contains("--cp")) {
				this.options.remove("--cp");
				this.options.add("-cp");
			}
			this.description = descriptor.description();
		}

		@Override
		public Set<String> getOptions() {
			return this.options;
		}

		@Override
		public String getUsageHelp() {
			return this.description;
		}

	}

}
