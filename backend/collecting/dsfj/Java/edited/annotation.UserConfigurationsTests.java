

package org.springframework.boot.context.annotation;

import java.io.InputStream;
import java.io.OutputStream;

import org.junit.Test;

import static org.assertj.core.api.Assertions.assertThat;


public class UserConfigurationsTests {

	@Test
	public void ofShouldCreateUnorderedConfigurations() {
		UserConfigurations configurations = UserConfigurations.of(OutputStream.class,
				InputStream.class);
		assertThat(Configurations.getClasses(configurations))
				.containsExactly(OutputStream.class, InputStream.class);
	}

}
