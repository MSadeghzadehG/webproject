

package org.springframework.boot.launcher.it.props;

import java.io.IOException;
import java.util.Properties;

import org.springframework.beans.factory.InitializingBean;
import org.springframework.context.annotation.ComponentScan;
import org.springframework.context.annotation.Configuration;
import org.springframework.core.io.ClassPathResource;


@Configuration
@ComponentScan
public class SpringConfiguration implements InitializingBean {

	private String message = "Jar";

	@Override
	public void afterPropertiesSet() throws IOException {
		Properties props = new Properties();
		props.load(new ClassPathResource("application.properties").getInputStream());
		String value = props.getProperty("message");
		if (value!=null) {
			this.message = value;
		}

	}

	public void run(String... args) {
		System.err.println("Hello Embedded " + this.message + "!");
	}

}
