

package org.springframework.boot.cli.command.options;

import java.io.File;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import joptsimple.OptionSet;

import org.springframework.boot.cli.util.ResourceUtils;
import org.springframework.util.Assert;
import org.springframework.util.StringUtils;


public class SourceOptions {

	private final List<String> sources;

	private final List<?> args;

	
	public SourceOptions(OptionSet options) {
		this(options, null);
	}

	
	public SourceOptions(List<?> arguments) {
		this(arguments, null);
	}

	
	public SourceOptions(OptionSet optionSet, ClassLoader classLoader) {
		this(optionSet.nonOptionArguments(), classLoader);
	}

	private SourceOptions(List<?> nonOptionArguments, ClassLoader classLoader) {
		List<String> sources = new ArrayList<>();
		int sourceArgCount = 0;
		for (Object option : nonOptionArguments) {
			if (option instanceof String) {
				String filename = (String) option;
				if ("--".equals(filename)) {
					break;
				}
				List<String> urls = new ArrayList<>();
				File fileCandidate = new File(filename);
				if (fileCandidate.isFile()) {
					urls.add(fileCandidate.getAbsoluteFile().toURI().toString());
				}
				else if (!isAbsoluteWindowsFile(fileCandidate)) {
					urls.addAll(ResourceUtils.getUrls(filename, classLoader));
				}
				for (String url : urls) {
					if (isSource(url)) {
						sources.add(url);
					}
				}
				if (isSource(filename)) {
					if (urls.isEmpty()) {
						throw new IllegalArgumentException("Can't find " + filename);
					}
					else {
						sourceArgCount++;
					}
				}
			}
		}
		this.args = Collections.unmodifiableList(
				nonOptionArguments.subList(sourceArgCount, nonOptionArguments.size()));
		Assert.isTrue(!sources.isEmpty(), "Please specify at least one file");
		this.sources = Collections.unmodifiableList(sources);
	}

	private boolean isAbsoluteWindowsFile(File file) {
		return isWindows() && file.isAbsolute();
	}

	private boolean isWindows() {
		return File.separatorChar == '\\';
	}

	private boolean isSource(String name) {
		return name.endsWith(".java") || name.endsWith(".groovy");
	}

	public List<?> getArgs() {
		return this.args;
	}

	public String[] getArgsArray() {
		return this.args.stream().map(this::asString).toArray(String[]::new);
	}

	private String asString(Object arg) {
		return (arg == null ? null : String.valueOf(arg));
	}

	public List<String> getSources() {
		return this.sources;
	}

	public String[] getSourcesArray() {
		return StringUtils.toStringArray(this.sources);
	}

}
