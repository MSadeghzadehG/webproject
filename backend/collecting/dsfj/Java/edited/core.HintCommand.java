

package org.springframework.boot.cli.command.core;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.springframework.boot.cli.command.AbstractCommand;
import org.springframework.boot.cli.command.Command;
import org.springframework.boot.cli.command.CommandRunner;
import org.springframework.boot.cli.command.options.OptionHelp;
import org.springframework.boot.cli.command.status.ExitStatus;
import org.springframework.boot.cli.util.Log;


public class HintCommand extends AbstractCommand {

	private final CommandRunner commandRunner;

	public HintCommand(CommandRunner commandRunner) {
		super("hint", "Provides hints for shell auto-completion");
		this.commandRunner = commandRunner;
	}

	@Override
	public ExitStatus run(String... args) throws Exception {
		try {
			int index = (args.length == 0 ? 0 : Integer.valueOf(args[0]) - 1);
			List<String> arguments = new ArrayList<>(args.length);
			for (int i = 2; i < args.length; i++) {
				arguments.add(args[i]);
			}
			String starting = "";
			if (index < arguments.size()) {
				starting = arguments.remove(index);
			}
			if (index == 0) {
				showCommandHints(starting);
			}
			else if (!arguments.isEmpty() && !starting.isEmpty()) {
				String command = arguments.remove(0);
				showCommandOptionHints(command, Collections.unmodifiableList(arguments),
						starting);
			}
		}
		catch (Exception ex) {
						return ExitStatus.ERROR;
		}
		return ExitStatus.OK;
	}

	private void showCommandHints(String starting) {
		for (Command command : this.commandRunner) {
			if (isHintMatch(command, starting)) {
				Log.info(command.getName() + " " + command.getDescription());
			}
		}
	}

	private boolean isHintMatch(Command command, String starting) {
		if (command instanceof HintCommand) {
			return false;
		}
		return command.getName().startsWith(starting)
				|| (this.commandRunner.isOptionCommand(command)
						&& ("--" + command.getName()).startsWith(starting));
	}

	private void showCommandOptionHints(String commandName,
			List<String> specifiedArguments, String starting) {
		Command command = this.commandRunner.findCommand(commandName);
		if (command != null) {
			for (OptionHelp help : command.getOptionsHelp()) {
				if (!alreadyUsed(help, specifiedArguments)) {
					for (String option : help.getOptions()) {
						if (option.startsWith(starting)) {
							Log.info(option + " " + help.getUsageHelp());
						}
					}
				}
			}
		}
	}

	private boolean alreadyUsed(OptionHelp help, List<String> specifiedArguments) {
		for (String argument : specifiedArguments) {
			if (help.getOptions().contains(argument)) {
				return true;
			}
		}
		return false;
	}

}
