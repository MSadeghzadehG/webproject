

package org.springframework.boot;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.junit.Rule;
import org.junit.Test;

import org.springframework.boot.testsupport.rule.OutputCapture;
import org.springframework.context.annotation.Configuration;
import org.springframework.util.ClassUtils;
import org.springframework.util.StringUtils;

import static org.assertj.core.api.Assertions.assertThat;


@Configuration
public class SimpleMainTests {

	@Rule
	public OutputCapture outputCapture = new OutputCapture();

	private static final String SPRING_STARTUP = "root of context hierarchy";

	@Test(expected = IllegalArgumentException.class)
	public void emptyApplicationContext() throws Exception {
		SpringApplication.main(getArgs());
		assertThat(getOutput()).contains(SPRING_STARTUP);
	}

	@Test
	public void basePackageScan() throws Exception {
		SpringApplication
				.main(getArgs(ClassUtils.getPackageName(getClass()) + ".sampleconfig"));
		assertThat(getOutput()).contains(SPRING_STARTUP);
	}

	@Test
	public void configClassContext() throws Exception {
		SpringApplication.main(getArgs(getClass().getName()));
		assertThat(getOutput()).contains(SPRING_STARTUP);
	}

	@Test
	public void xmlContext() throws Exception {
		SpringApplication.main(getArgs("org/springframework/boot/sample-beans.xml"));
		assertThat(getOutput()).contains(SPRING_STARTUP);
	}

	@Test
	public void mixedContext() throws Exception {
		SpringApplication.main(getArgs(getClass().getName(),
				"org/springframework/boot/sample-beans.xml"));
		assertThat(getOutput()).contains(SPRING_STARTUP);
	}

	private String[] getArgs(String... args) {
		List<String> list = new ArrayList<>(Arrays.asList(
				"--spring.main.webEnvironment=false", "--spring.main.showBanner=OFF",
				"--spring.main.registerShutdownHook=false"));
		if (args.length > 0) {
			list.add("--spring.main.sources="
					+ StringUtils.arrayToCommaDelimitedString(args));
		}
		return StringUtils.toStringArray(list);
	}

	private String getOutput() {
		return this.outputCapture.toString();
	}

}
