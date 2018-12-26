

package org.springframework.boot.cli.command;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.EnumSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import org.springframework.boot.cli.command.status.ExitStatus;
import org.springframework.boot.cli.util.Log;
import org.springframework.util.Assert;
import org.springframework.util.StringUtils;


public class CommandRunner implements Iterable<Command> {

	private static final Set<CommandException.Option> NO_EXCEPTION_OPTIONS = EnumSet
			.noneOf(CommandException.Option.class);

	private final String name;

	private final List<Command> commands = new ArrayList<>();

	private Class<?>[] optionCommandClasses = {};

	private Class<?>[] hiddenCommandClasses = {};

	
	public CommandRunner(String name) {
		this.name = (StringUtils.hasLength(name) ? name + " " : "");
	}

	
	public String getName() {
		return this.name;
	}

	
	public void addCommands(Iterable<Command> commands) {
		Assert.notNull(commands, "Commands must not be null");
		for (Command command : commands) {
			addCommand(command);
		}
	}

	
	public void addCommand(Command command) {
		Assert.notNull(command, "Command must not be null");
		this.commands.add(command);
	}

	
	public void setOptionCommands(Class<?>... commandClasses) {
		Assert.notNull(commandClasses, "CommandClasses must not be null");
		this.optionCommandClasses = commandClasses;
	}

	
	public void setHiddenCommands(Class<?>... commandClasses) {
		Assert.notNull(commandClasses, "CommandClasses must not be null");
		this.hiddenCommandClasses = commandClasses;
	}

	
	public boolean isOptionCommand(Command command) {
		return isCommandInstanceOf(command, this.optionCommandClasses);
	}

	private boolean isHiddenCommand(Command command) {
		return isCommandInstanceOf(command, this.hiddenCommandClasses);
	}

	private boolean isCommandInstanceOf(Command command, Class<?>[] commandClasses) {
		for (Class<?> commandClass : commandClasses) {
			if (commandClass.isInstance(command)) {
				return true;
			}
		}
		return false;
	}

	@Override
	public Iterator<Command> iterator() {
		return getCommands().iterator();
	}

	protected final List<Command> getCommands() {
		return Collections.unmodifiableList(this.commands);
	}

	
	public Command findCommand(String name) {
		for (Command candidate : this.commands) {
			String candidateName = candidate.getName();
			if (candidateName.equals(name) || (isOptionCommand(candidate)
					&& ("--" + candidateName).equals(name))) {
				return candidate;
			}
		}
		return null;
	}

	
	public int runAndHandleErrors(String... args) {
		String[] argsWithoutDebugFlags = removeDebugFlags(args);
		boolean debug = argsWithoutDebugFlags.length != args.length;
		if (debug) {
			System.setProperty("debug", "true");
		}
		try {
			ExitStatus result = run(argsWithoutDebugFlags);
						if (result != null && result.isHangup()) {
				return (result.getCode() > 0 ? result.getCode() : 0);
			}
			return 0;
		}
		catch (NoArgumentsException ex) {
			showUsage();
			return 1;
		}
		catch (Exception ex) {
			return handleError(debug, ex);
		}
	}

	private String[] removeDebugFlags(String[] args) {
		List<String> rtn = new ArrayList<>(args.length);
		boolean appArgsDetected = false;
		for (String arg : args) {
						appArgsDetected |= "--".equals(arg);
			if (("-d".equals(arg) || "--debug".equals(arg)) && !appArgsDetected) {
				continue;
			}
			rtn.add(arg);
		}
		return StringUtils.toStringArray(rtn);
	}

	
	protected ExitStatus run(String... args) throws Exception {
		if (args.length == 0) {
			throw new NoArgumentsException();
		}
		String commandName = args[0];
		String[] commandArguments = Arrays.copyOfRange(args, 1, args.length);
		Command command = findCommand(commandName);
		if (command == null) {
			throw new NoSuchCommandException(commandName);
		}
		beforeRun(command);
		try {
			return command.run(commandArguments);
		}
		finally {
			afterRun(command);
		}
	}

	
	protected void beforeRun(Command command) {
	}

	
	protected void afterRun(Command command) {
	}

	private int handleError(boolean debug, Exception ex) {
		Set<CommandException.Option> options = NO_EXCEPTION_OPTIONS;
		if (ex instanceof CommandException) {
			options = ((CommandException) ex).getOptions();
			if (options.contains(CommandException.Option.RETHROW)) {
				throw (CommandException) ex;
			}
		}
		boolean couldNotShowMessage = false;
		if (!options.contains(CommandException.Option.HIDE_MESSAGE)) {
			couldNotShowMessage = !errorMessage(ex.getMessage());
		}
		if (options.contains(CommandException.Option.SHOW_USAGE)) {
			showUsage();
		}
		if (debug || couldNotShowMessage
				|| options.contains(CommandException.Option.STACK_TRACE)) {
			printStackTrace(ex);
		}
		return 1;
	}

	protected boolean errorMessage(String message) {
		Log.error(message == null ? "Unexpected error" : message);
		return message != null;
	}

	protected void showUsage() {
		Log.infoPrint("usage: " + this.name);
		for (Command command : this.commands) {
			if (isOptionCommand(command)) {
				Log.infoPrint("[--" + command.getName() + "] ");
			}
		}
		Log.info("");
		Log.info("       <command> [<args>]");
		Log.info("");
		Log.info("Available commands are:");
		for (Command command : this.commands) {
			if (!isOptionCommand(command) && !isHiddenCommand(command)) {
				String usageHelp = command.getUsageHelp();
				String description = command.getDescription();
				Log.info(String.format("%n  %1$s %2$-15s%n    %3$s", command.getName(),
						(usageHelp == null ? "" : usageHelp),
						(description == null ? "" : description)));
			}
		}
		Log.info("");
		Log.info("Common options:");
		Log.info(String.format("%n  %1$s %2$-15s%n    %3$s", "-d, --debug",
				"Verbose mode",
				"Print additional status information for the command you are running"));
		Log.info("");
		Log.info("");
		Log.info("See '" + this.name
				+ "help <command>' for more information on a specific command.");
	}

	protected void printStackTrace(Exception ex) {
		Log.error("");
		Log.error(ex);
		Log.error("");
	}

}
