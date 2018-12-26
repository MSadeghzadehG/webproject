

package org.springframework.boot.cli.command.shell;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.ServiceLoader;
import java.util.Set;

import jline.console.ConsoleReader;
import jline.console.completer.CandidateListCompletionHandler;
import org.fusesource.jansi.AnsiRenderer.Code;

import org.springframework.boot.cli.command.Command;
import org.springframework.boot.cli.command.CommandFactory;
import org.springframework.boot.cli.command.CommandRunner;
import org.springframework.boot.cli.command.core.HelpCommand;
import org.springframework.boot.cli.command.core.VersionCommand;
import org.springframework.boot.loader.tools.SignalUtils;
import org.springframework.util.StringUtils;


public class Shell {

	private static final Set<Class<?>> NON_FORKED_COMMANDS;

	static {
		Set<Class<?>> nonForked = new HashSet<>();
		nonForked.add(VersionCommand.class);
		NON_FORKED_COMMANDS = Collections.unmodifiableSet(nonForked);
	}

	private final ShellCommandRunner commandRunner;

	private final ConsoleReader consoleReader;

	private final EscapeAwareWhiteSpaceArgumentDelimiter argumentDelimiter = new EscapeAwareWhiteSpaceArgumentDelimiter();

	private final ShellPrompts prompts = new ShellPrompts();

	
	Shell() throws IOException {
		attachSignalHandler();
		this.consoleReader = new ConsoleReader();
		this.commandRunner = createCommandRunner();
		initializeConsoleReader();
	}

	private ShellCommandRunner createCommandRunner() {
		ShellCommandRunner runner = new ShellCommandRunner();
		runner.addCommand(new HelpCommand(runner));
		runner.addCommands(getCommands());
		runner.addAliases("exit", "quit");
		runner.addAliases("help", "?");
		runner.addAliases("clear", "cls");
		return runner;
	}

	private Iterable<Command> getCommands() {
		List<Command> commands = new ArrayList<>();
		ServiceLoader<CommandFactory> factories = ServiceLoader.load(CommandFactory.class,
				getClass().getClassLoader());
		for (CommandFactory factory : factories) {
			for (Command command : factory.getCommands()) {
				commands.add(convertToForkCommand(command));
			}
		}
		commands.add(new PromptCommand(this.prompts));
		commands.add(new ClearCommand(this.consoleReader));
		commands.add(new ExitCommand());
		return commands;
	}

	private Command convertToForkCommand(Command command) {
		for (Class<?> nonForked : NON_FORKED_COMMANDS) {
			if (nonForked.isInstance(command)) {
				return command;
			}
		}
		return new ForkProcessCommand(command);
	}

	private void initializeConsoleReader() {
		this.consoleReader.setHistoryEnabled(true);
		this.consoleReader.setBellEnabled(false);
		this.consoleReader.setExpandEvents(false);
		this.consoleReader.addCompleter(new CommandCompleter(this.consoleReader,
				this.argumentDelimiter, this.commandRunner));
		this.consoleReader.setCompletionHandler(new CandidateListCompletionHandler());
	}

	private void attachSignalHandler() {
		SignalUtils.attachSignalHandler(this::handleSigInt);
	}

	
	public void run() throws Exception {
		printBanner();
		try {
			runInputLoop();
		}
		catch (Exception ex) {
			if (!(ex instanceof ShellExitException)) {
				throw ex;
			}
		}
	}

	private void printBanner() {
		String version = getClass().getPackage().getImplementationVersion();
		version = (version == null ? "" : " (v" + version + ")");
		System.out.println(ansi("Spring Boot", Code.BOLD).append(version, Code.FAINT));
		System.out.println(ansi("Hit TAB to complete. Type 'help' and hit "
				+ "RETURN for help, and 'exit' to quit."));
	}

	private void runInputLoop() throws Exception {
		String line;
		while ((line = this.consoleReader.readLine(getPrompt())) != null) {
			while (line.endsWith("\\")) {
				line = line.substring(0, line.length() - 1);
				line += this.consoleReader.readLine("> ");
			}
			if (StringUtils.hasLength(line)) {
				String[] args = this.argumentDelimiter.parseArguments(line);
				this.commandRunner.runAndHandleErrors(args);
			}
		}
	}

	private String getPrompt() {
		String prompt = this.prompts.getPrompt();
		return ansi(prompt, Code.FG_BLUE).toString();
	}

	private AnsiString ansi(String text, Code... codes) {
		return new AnsiString(this.consoleReader.getTerminal()).append(text, codes);
	}

	
	protected void handleSigInt() {
		if (this.commandRunner.handleSigInt()) {
			return;
		}
		System.out.println(String.format("%nThanks for using Spring Boot"));
		System.exit(1);
	}

	
	private class ShellCommandRunner extends CommandRunner {

		private volatile Command lastCommand;

		private final Map<String, String> aliases = new HashMap<>();

		ShellCommandRunner() {
			super(null);
		}

		public void addAliases(String command, String... aliases) {
			for (String alias : aliases) {
				this.aliases.put(alias, command);
			}
		}

		@Override
		public Command findCommand(String name) {
			if (name.startsWith("!")) {
				return new RunProcessCommand(name.substring(1));
			}
			if (this.aliases.containsKey(name)) {
				name = this.aliases.get(name);
			}
			return super.findCommand(name);
		}

		@Override
		protected void beforeRun(Command command) {
			this.lastCommand = command;
		}

		@Override
		protected void afterRun(Command command) {
		}

		public boolean handleSigInt() {
			Command command = this.lastCommand;
			if (command != null && command instanceof RunProcessCommand) {
				return ((RunProcessCommand) command).handleSigInt();
			}
			return false;
		}

	}

}
