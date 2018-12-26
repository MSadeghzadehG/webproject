

package org.springframework.boot.maven;

import java.util.Arrays;
import java.util.Deque;
import java.util.LinkedList;
import java.util.Objects;

import org.codehaus.plexus.util.cli.CommandLineUtils;


class RunArguments {

	private static final String[] NO_ARGS = {};

	private final LinkedList<String> args = new LinkedList<>();

	RunArguments(String arguments) {
		this(parseArgs(arguments));
	}

	RunArguments(String[] args) {
		if (args != null) {
			Arrays.stream(args).filter(Objects::nonNull).forEach(this.args::add);
		}
	}

	public Deque<String> getArgs() {
		return this.args;
	}

	public String[] asArray() {
		return this.args.toArray(new String[0]);
	}

	private static String[] parseArgs(String arguments) {
		if (arguments == null || arguments.trim().isEmpty()) {
			return NO_ARGS;
		}
		try {
			arguments = arguments.replace('\n', ' ').replace('\t', ' ');
			return CommandLineUtils.translateCommandline(arguments);
		}
		catch (Exception ex) {
			throw new IllegalArgumentException(
					"Failed to parse arguments [" + arguments + "]", ex);
		}
	}

}
