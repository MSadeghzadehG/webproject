

package org.springframework.boot.cli.command;

import java.util.Collection;
import java.util.Collections;

import org.springframework.boot.cli.command.options.OptionHelp;


public abstract class AbstractCommand implements Command {

	private final String name;

	private final String description;

	
	protected AbstractCommand(String name, String description) {
		this.name = name;
		this.description = description;
	}

	@Override
	public String getName() {
		return this.name;
	}

	@Override
	public String getDescription() {
		return this.description;
	}

	@Override
	public String getUsageHelp() {
		return null;
	}

	@Override
	public String getHelp() {
		return null;
	}

	@Override
	public Collection<OptionHelp> getOptionsHelp() {
		return Collections.emptyList();
	}

	@Override
	public Collection<HelpExample> getExamples() {
		return null;
	}

}
