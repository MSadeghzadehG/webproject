

package org.springframework.boot.devtools.tests;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import org.springframework.boot.devtools.tests.JvmLauncher.LaunchedJvm;
import org.springframework.util.FileSystemUtils;
import org.springframework.util.StringUtils;


public class LocalApplicationLauncher implements ApplicationLauncher {

	@Override
	public LaunchedApplication launchApplication(JvmLauncher jvmLauncher)
			throws Exception {
		LaunchedJvm jvm = jvmLauncher.launch("local", createApplicationClassPath(),
				"com.example.DevToolsTestApplication", "--server.port=0");
		return new LaunchedApplication(new File("target/app"), jvm.getStandardOut(),
				jvm.getStandardError(), jvm.getProcess());
	}

	protected String createApplicationClassPath() throws Exception {
		File appDirectory = new File("target/app");
		FileSystemUtils.deleteRecursively(appDirectory);
		appDirectory.mkdirs();
		FileSystemUtils.copyRecursively(new File("target/test-classes/com"),
				new File("target/app/com"));
		List<String> entries = new ArrayList<>();
		entries.add("target/app");
		for (File jar : new File("target/dependencies").listFiles()) {
			entries.add(jar.getAbsolutePath());
		}
		return StringUtils.collectionToDelimitedString(entries, File.pathSeparator);
	}

	@Override
	public String toString() {
		return "local";
	}

}
