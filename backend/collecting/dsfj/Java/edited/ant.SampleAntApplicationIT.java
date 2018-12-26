

package sample.ant;

import static org.assertj.core.api.Assertions.assertThat;

import java.io.File;
import java.io.FileFilter;
import java.io.InputStreamReader;
import java.util.concurrent.TimeUnit;

import org.junit.Test;
import org.springframework.boot.loader.tools.JavaExecutable;
import org.springframework.util.FileCopyUtils;


public class SampleAntApplicationIT {

	@Test
	public void runJar() throws Exception {
		File target = new File("target");
		File[] jarFiles = target.listFiles(new FileFilter() {

			@Override
			public boolean accept(File file) {
				return file.getName().endsWith(".jar");
			}

		});
		assertThat(jarFiles).hasSize(1);
		Process process = new JavaExecutable().processBuilder("-jar", jarFiles[0]
				.getName()).directory(target).start();
		process.waitFor(5, TimeUnit.MINUTES);
		assertThat(process.exitValue()).isEqualTo(0);
		String output = FileCopyUtils.copyToString(new InputStreamReader(process
				.getInputStream()));
		assertThat(output).contains("Spring Boot Ant Example");
	}

}
