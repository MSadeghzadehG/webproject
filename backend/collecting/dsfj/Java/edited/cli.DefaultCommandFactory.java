

package org.springframework.boot.cli;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import org.springframework.boot.cli.command.Command;
import org.springframework.boot.cli.command.CommandFactory;
import org.springframework.boot.cli.command.archive.JarCommand;
import org.springframework.boot.cli.command.archive.WarCommand;
import org.springframework.boot.cli.command.core.VersionCommand;
import org.springframework.boot.cli.command.encodepassword.EncodePasswordCommand;
import org.springframework.boot.cli.command.grab.GrabCommand;
import org.springframework.boot.cli.command.init.InitCommand;
import org.springframework.boot.cli.command.install.InstallCommand;
import org.springframework.boot.cli.command.install.UninstallCommand;
import org.springframework.boot.cli.command.run.RunCommand;


public class DefaultCommandFactory implements CommandFactory {

	private static final List<Command> DEFAULT_COMMANDS;

	static {
		List<Command> defaultCommands = new ArrayList<>();
		defaultCommands.add(new VersionCommand());
		defaultCommands.add(new RunCommand());
		defaultCommands.add(new GrabCommand());
		defaultCommands.add(new JarCommand());
		defaultCommands.add(new WarCommand());
		defaultCommands.add(new InstallCommand());
		defaultCommands.add(new UninstallCommand());
		defaultCommands.add(new InitCommand());
		defaultCommands.add(new EncodePasswordCommand());
		DEFAULT_COMMANDS = Collections.unmodifiableList(defaultCommands);
	}

	@Override
	public Collection<Command> getCommands() {
		return DEFAULT_COMMANDS;
	}

}
