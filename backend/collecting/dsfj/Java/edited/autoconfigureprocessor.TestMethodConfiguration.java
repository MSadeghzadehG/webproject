

package org.springframework.boot.autoconfigureprocessor;

import java.io.OutputStream;


@TestConfiguration
public class TestMethodConfiguration {

	@TestConditionalOnClass(name = "java.io.InputStream", value = OutputStream.class)
	public Object method() {
		return null;
	}
}
