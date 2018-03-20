

package org.springframework.boot.loader.tools;

import java.io.File;
import java.io.IOException;
import java.util.Arrays;
import java.util.Collection;


public class RunProcess {

	private static final long JUST_ENDED_LIMIT = 500;

	private File workingDirectory;

	private final String[] command;

	private volatile Process process;

	private volatile long endTime;

	
	public RunProcess(String... command) {
		this(null, command);
	}

	
	public RunProcess(File workingDirectory, String... command) {
		this.workingDirectory = workingDirectory;
		this.command = command;
	}

	public int run(boolean waitForProcess, String... args) throws IOException {
		return run(waitForProcess, Arrays.asList(args));
	}

	protected int run(boolean waitForProcess, Collection<String> args)
			throws IOException {
		ProcessBuilder builder = new ProcessBuilder(this.command);
		builder.directory(this.workingDirectory);
		builder.command().addAll(args);
		builder.redirectErrorStream(true);
		builder.inheritIO();
		try {
			Process process = builder.start();
			this.process = process;
			SignalUtils.attachSignalHandler(this::handleSigInt);
			if (waitForProcess) {
				try {
					return process.waitFor();
				}
				catch (InterruptedException ex) {
					Thread.currentThread().interrupt();
					return 1;
				}
			}
			return 5;
		}
		finally {
			if (waitForProcess) {
				this.endTime = System.currentTimeMillis();
				this.process = null;
			}
		}
	}

	
	public Process getRunningProcess() {
		return this.process;
	}

	
	public boolean handleSigInt() {
				if (hasJustEnded()) {
			return true;
		}
		return doKill();
	}

	
	public void kill() {
		doKill();
	}

	private boolean doKill() {
				Process process = this.process;
		if (process != null) {
			try {
				process.destroy();
				process.waitFor();
				this.process = null;
				return true;
			}
			catch (InterruptedException ex) {
				Thread.currentThread().interrupt();
			}
		}
		return false;
	}

	public boolean hasJustEnded() {
		return System.currentTimeMillis() < (this.endTime + JUST_ENDED_LIMIT);
	}

}
