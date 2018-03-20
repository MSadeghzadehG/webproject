

package org.springframework.boot.cli.command.shell;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;

import org.springframework.boot.cli.command.Command;
import org.springframework.boot.cli.command.options.OptionHelp;
import org.springframework.boot.cli.command.status.ExitStatus;
import org.springframework.boot.loader.tools.JavaExecutable;


class ForkProcessCommand extends RunProcessCommand {

	private static final String MAIN_CLASS = "org.springframework.boot.loader.JarLauncher";

	private final Command command;

	ForkProcessCommand(Command command) {
		super(new JavaExecutable().toString());
		this.command = command;
	}

	@Override
	public String getName() {
		return this.command.getName();
	}

	@Override
	public String getDescription() {
		return this.command.getDescription();
	}

	@Override
	public String getUsageHelp() {
		return this.command.getUsageHelp();
	}

	@Override
	public String getHelp() {
		return this.command.getHelp();
	}

	@Override
	public Collection<OptionHelp> getOptionsHelp() {
		return this.command.getOptionsHelp();
	}

	@Override
	public ExitStatus run(String... args) throws Exception {
		List<String> fullArgs = new ArrayList<>();
		fullArgs.add("-cp");
		fullArgs.add(System.getProperty("java.class.path"));
		fullArgs.add(MAIN_CLASS);
		fullArgs.add(this.command.getName());
		fullArgs.addAll(Arrays.asList(args));
		run(fullArgs);
		return ExitStatus.OK;
	}

}
