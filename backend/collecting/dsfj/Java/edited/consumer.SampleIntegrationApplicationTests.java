

package sample.integration.consumer;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.util.concurrent.Callable;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import sample.integration.SampleIntegrationApplication;
import sample.integration.producer.ProducerApplication;

import org.springframework.boot.SpringApplication;
import org.springframework.context.ConfigurableApplicationContext;
import org.springframework.core.io.DefaultResourceLoader;
import org.springframework.core.io.Resource;
import org.springframework.core.io.support.ResourcePatternUtils;
import org.springframework.util.FileSystemUtils;
import org.springframework.util.StreamUtils;

import static org.assertj.core.api.Assertions.assertThat;


public class SampleIntegrationApplicationTests {

	private ConfigurableApplicationContext context;

	@Before
	public void deleteInputAndOutput() {
		deleteIfExists(new File("target/input"));
		deleteIfExists(new File("target/output"));
	}

	private void deleteIfExists(File directory) {
		if (directory.exists()) {
			assertThat(FileSystemUtils.deleteRecursively(directory)).isTrue();
		}
	}

	@After
	public void stop() {
		if (this.context != null) {
			this.context.close();
		}
	}

	@Test
	public void testVanillaExchange() throws Exception {
		this.context = SpringApplication.run(SampleIntegrationApplication.class);
		SpringApplication.run(ProducerApplication.class, "World");
		String output = getOutput();
		assertThat(output).contains("Hello World");
	}

	@Test
	public void testMessageGateway() throws Exception {
		this.context = SpringApplication.run(SampleIntegrationApplication.class,
				"testviamg");
		String output = getOutput();
		assertThat(output).contains("testviamg");
	}

	private String getOutput() throws Exception {
		Future<String> future = Executors.newSingleThreadExecutor()
				.submit(new Callable<String>() {
					@Override
					public String call() throws Exception {
						Resource[] resources = getResourcesWithContent();
						while (resources.length == 0) {
							Thread.sleep(200);
							resources = getResourcesWithContent();
						}
						StringBuilder builder = new StringBuilder();
						for (Resource resource : resources) {
							try (InputStream inputStream = resource.getInputStream()) {
								builder.append(new String(
										StreamUtils.copyToByteArray(inputStream)));
							}
						}
						return builder.toString();
					}
				});
		return future.get(30, TimeUnit.SECONDS);
	}

	private Resource[] getResourcesWithContent() throws IOException {
		Resource[] candidates = ResourcePatternUtils
				.getResourcePatternResolver(new DefaultResourceLoader())
				.getResources("file:target/output