

package org.springframework.boot.cli.command.shell;

import java.io.IOException;
import java.util.Arrays;
import java.util.Collection;

import org.springframework.boot.cli.command.AbstractCommand;
import org.springframework.boot.cli.command.Command;
import org.springframework.boot.cli.command.status.ExitStatus;
import org.springframework.boot.loader.tools.RunProcess;
import org.springframework.util.StringUtils;


class RunProcessCommand extends AbstractCommand {

	private final String[] command;

	private volatile RunProcess process;

	RunProcessCommand(String... command) {
		super(null, null);
		this.command = command;
	}

	@Override
	public ExitStatus run(String... args) throws Exception {
		return run(Arrays.asList(args));
	}

	protected ExitStatus run(Collection<String> args) throws IOException {
		this.process = new RunProcess(this.command);
		int code = this.process.run(true, StringUtils.toStringArray(args));
		if (code == 0) {
			return ExitStatus.OK;
		}
		else {
			return new ExitStatus(code, "EXTERNAL_ERROR");
		}
	}

	public boolean handleSigInt() {
		return this.process.handleSigInt();
	}

}
