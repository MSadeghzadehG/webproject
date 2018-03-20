

package org.springframework.boot.test.autoconfigure;

import java.io.PrintWriter;
import java.io.StringWriter;
import java.util.ArrayList;
import java.util.List;

import javax.annotation.PostConstruct;

import org.junit.Test;
import org.junit.runner.RunWith;

import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.context.annotation.Configuration;
import org.springframework.test.context.junit4.SpringRunner;

import static org.assertj.core.api.Assertions.assertThat;


@RunWith(SpringRunner.class)
@SpringBootTest
public class SpringBootDependencyInjectionTestExecutionListenerPostConstructIntegrationTests {

	private List<String> calls = new ArrayList<>();

	@PostConstruct
	public void postConstruct() {
		StringWriter writer = new StringWriter();
		new RuntimeException().printStackTrace(new PrintWriter(writer));
		this.calls.add(writer.toString());
	}

	@Test
	public void postConstructShouldBeInvokedOnlyOnce() {
				assertThat(this.calls).hasSize(1);
	}

	@Configuration
	static class Config {

	}

}
