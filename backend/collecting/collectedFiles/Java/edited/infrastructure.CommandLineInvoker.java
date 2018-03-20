

package org.springframework.boot.cli.infrastructure;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.PrintWriter;
import java.io.StringReader;
import java.io.StringWriter;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.stream.Collectors;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

import org.springframework.util.Assert;
import org.springframework.util.StreamUtils;


public final class CommandLineInvoker {

	private final File workingDirectory;

	public CommandLineInvoker() {
		this(new File("."));
	}

	public CommandLineInvoker(File workingDirectory) {
		this.workingDirectory = workingDirectory;
	}

	public Invocation invoke(String... args) throws IOException {
		return new Invocation(runCliProcess(args));
	}

	private Process runCliProcess(String... args) throws IOException {
		List<String> command = new ArrayList<>();
		command.add(findLaunchScript().getAbsolutePath());
		command.addAll(Arrays.asList(args));
		ProcessBuilder processBuilder = new ProcessBuilder(command)
				.directory(this.workingDirectory);
		processBuilder.environment().remove("JAVA_OPTS");
		return processBuilder.start();
	}

	private File findLaunchScript() throws IOException {
		File unpacked = new File("target/unpacked-cli");
		if (!unpacked.isDirectory()) {
			File zip = new File("target")
					.listFiles((pathname) -> pathname.getName().endsWith("-bin.zip"))[0];
			try (ZipInputStream input = new ZipInputStream(new FileInputStream(zip))) {
				ZipEntry entry;
				while ((entry = input.getNextEntry()) != null) {
					File file = new File(unpacked, entry.getName());
					if (entry.isDirectory()) {
						file.mkdirs();
					}
					else {
						file.getParentFile().mkdirs();
						try (FileOutputStream output = new FileOutputStream(file)) {
							StreamUtils.copy(input, output);
							if (entry.getName().endsWith("/bin/spring")) {
								file.setExecutable(true);
							}
						}
					}
				}
			}
		}
		File bin = new File(unpacked.listFiles()[0], "bin");
		File launchScript = new File(bin, isWindows() ? "spring.bat" : "spring");
		Assert.state(launchScript.exists() && launchScript.isFile(),
				() -> "Could not find CLI launch script "
						+ launchScript.getAbsolutePath());
		return launchScript;
	}

	private boolean isWindows() {
		return File.separatorChar == '\\';
	}

	
	public static final class Invocation {

		private final StringBuffer err = new StringBuffer();

		private final StringBuffer out = new StringBuffer();

		private final StringBuffer combined = new StringBuffer();

		private final Process process;

		private final List<Thread> streamReaders = new ArrayList<>();

		public Invocation(Process process) {
			this.process = process;
			this.streamReaders.add(new Thread(new StreamReadingRunnable(
					this.process.getErrorStream(), this.err, this.combined)));
			this.streamReaders.add(new Thread(new StreamReadingRunnable(
					this.process.getInputStream(), this.out, this.combined)));
			for (Thread streamReader : this.streamReaders) {
				streamReader.start();
			}
		}

		public String getOutput() {
			return postProcessLines(getLines(this.combined));
		}

		public String getErrorOutput() {
			return postProcessLines(getLines(this.err));
		}

		public String getStandardOutput() {
			return postProcessLines(getStandardOutputLines());
		}

		public List<String> getStandardOutputLines() {
			return getLines(this.out);
		}

		private String postProcessLines(List<String> lines) {
			StringWriter out = new StringWriter();
			PrintWriter printOut = new PrintWriter(out);
			for (String line : lines) {
				if (!line.startsWith("Maven settings decryption failed")) {
					printOut.println(line);
				}
			}
			return out.toString();
		}

		private List<String> getLines(StringBuffer buffer) {
			BufferedReader reader = new BufferedReader(
					new StringReader(buffer.toString()));
			return reader.lines().filter((line) -> !line.startsWith("Picked up "))
					.collect(Collectors.toList());
		}

		public int await() throws InterruptedException {
			for (Thread streamReader : this.streamReaders) {
				streamReader.join();
			}
			return this.process.waitFor();
		}

		
		private final class StreamReadingRunnable implements Runnable {

			private final InputStream stream;

			private final StringBuffer[] outputs;

			private final byte[] buffer = new byte[4096];

			private StreamReadingRunnable(InputStream stream, StringBuffer... outputs) {
				this.stream = stream;
				this.outputs = outputs;
			}

			@Override
			public void run() {
				int read;
				try {
					while ((read = this.stream.read(this.buffer)) > 0) {
						for (StringBuffer output : this.outputs) {
							output.append(new String(this.buffer, 0, read));
						}
					}
				}
				catch (IOException ex) {
									}
			}

		}

	}

}
