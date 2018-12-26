

package org.springframework.boot.cli;

import java.io.File;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLClassLoader;
import java.util.ArrayList;
import java.util.List;
import java.util.ServiceLoader;

import org.springframework.boot.cli.command.CommandFactory;
import org.springframework.boot.cli.command.CommandRunner;
import org.springframework.boot.cli.command.core.HelpCommand;
import org.springframework.boot.cli.command.core.HintCommand;
import org.springframework.boot.cli.command.core.VersionCommand;
import org.springframework.boot.cli.command.shell.ShellCommand;
import org.springframework.boot.loader.tools.LogbackInitializer;
import org.springframework.util.ClassUtils;
import org.springframework.util.SystemPropertyUtils;


public final class SpringCli {

	private SpringCli() {
	}

	public static void main(String... args) {
		System.setProperty("java.awt.headless", Boolean.toString(true));
		LogbackInitializer.initialize();

		CommandRunner runner = new CommandRunner("spring");
		ClassUtils.overrideThreadContextClassLoader(createExtendedClassLoader(runner));
		runner.addCommand(new HelpCommand(runner));
		addServiceLoaderCommands(runner);
		runner.addCommand(new ShellCommand());
		runner.addCommand(new HintCommand(runner));
		runner.setOptionCommands(HelpCommand.class, VersionCommand.class);
		runner.setHiddenCommands(HintCommand.class);

		int exitCode = runner.runAndHandleErrors(args);
		if (exitCode != 0) {
						System.exit(exitCode);
		}
	}

	private static void addServiceLoaderCommands(CommandRunner runner) {
		ServiceLoader<CommandFactory> factories = ServiceLoader
				.load(CommandFactory.class);
		for (CommandFactory factory : factories) {
			runner.addCommands(factory.getCommands());
		}
	}

	private static URLClassLoader createExtendedClassLoader(CommandRunner runner) {
		return new URLClassLoader(getExtensionURLs(), runner.getClass().getClassLoader());
	}

	private static URL[] getExtensionURLs() {
		List<URL> urls = new ArrayList<>();
		String home = SystemPropertyUtils
				.resolvePlaceholders("${spring.home:${SPRING_HOME:.}}");
		File extDirectory = new File(new File(home, "lib"), "ext");
		if (extDirectory.isDirectory()) {
			for (File file : extDirectory.listFiles()) {
				if (file.getName().endsWith(".jar")) {
					try {
						urls.add(file.toURI().toURL());
					}
					catch (MalformedURLException ex) {
						throw new IllegalStateException(ex);
					}
				}
			}
		}
		return urls.toArray(new URL[0]);
	}

}
