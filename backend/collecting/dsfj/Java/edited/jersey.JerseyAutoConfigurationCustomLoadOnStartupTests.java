

package org.springframework.boot.autoconfigure.jersey;

import java.lang.annotation.Documented;
import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

import org.glassfish.jersey.server.ResourceConfig;
import org.junit.Test;
import org.junit.runner.RunWith;

import org.springframework.beans.DirectFieldAccessor;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.autoconfigure.context.PropertyPlaceholderAutoConfiguration;
import org.springframework.boot.autoconfigure.web.servlet.ServletWebServerFactoryAutoConfiguration;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.context.SpringBootTest.WebEnvironment;
import org.springframework.context.ApplicationContext;
import org.springframework.context.annotation.Configuration;
import org.springframework.context.annotation.Import;
import org.springframework.test.annotation.DirtiesContext;
import org.springframework.test.context.junit4.SpringRunner;

import static org.assertj.core.api.Assertions.assertThat;


@RunWith(SpringRunner.class)
@SpringBootTest(webEnvironment = WebEnvironment.RANDOM_PORT, properties = "spring.jersey.servlet.load-on-startup=5")
@DirtiesContext
public class JerseyAutoConfigurationCustomLoadOnStartupTests {

	@Autowired
	private ApplicationContext context;

	@Test
	public void contextLoads() {
		assertThat(
				new DirectFieldAccessor(this.context.getBean("jerseyServletRegistration"))
						.getPropertyValue("loadOnStartup")).isEqualTo(5);
	}

	@MinimalWebConfiguration
	public static class Application extends ResourceConfig {

		public Application() {
			register(Application.class);
		}

	}

	@Target(ElementType.TYPE)
	@Retention(RetentionPolicy.RUNTIME)
	@Documented
	@Configuration
	@Import({ ServletWebServerFactoryAutoConfiguration.class,
			JerseyAutoConfiguration.class, PropertyPlaceholderAutoConfiguration.class })
	protected @interface MinimalWebConfiguration {

	}

}
