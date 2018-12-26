

package org.springframework.boot.cli.command;

import java.util.Arrays;
import java.util.Collections;
import java.util.EnumSet;
import java.util.Set;


public class CommandException extends RuntimeException {

	private static final long serialVersionUID = 0L;

	private final EnumSet<Option> options;

	
	public CommandException(Option... options) {
		this.options = asEnumSet(options);
	}

	
	public CommandException(String message, Option... options) {
		super(message);
		this.options = asEnumSet(options);
	}

	
	public CommandException(String message, Throwable cause, Option... options) {
		super(message, cause);
		this.options = asEnumSet(options);
	}

	
	public CommandException(Throwable cause, Option... options) {
		super(cause);
		this.options = asEnumSet(options);
	}

	private EnumSet<Option> asEnumSet(Option[] options) {
		if (options == null || options.length == 0) {
			return EnumSet.noneOf(Option.class);
		}
		return EnumSet.copyOf(Arrays.asList(options));
	}

	
	public Set<Option> getOptions() {
		return Collections.unmodifiableSet(this.options);
	}

	
	public enum Option {

		
		HIDE_MESSAGE,

		
		SHOW_USAGE,

		
		STACK_TRACE,

		
		RETHROW

	}

}
