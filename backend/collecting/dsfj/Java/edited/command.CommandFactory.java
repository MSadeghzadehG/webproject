

package org.springframework.boot.cli.command;

import java.util.Collection;
import java.util.ServiceLoader;


@FunctionalInterface
public interface CommandFactory {

	
	Collection<Command> getCommands();

}
