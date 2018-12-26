

package org.springframework.boot.autoconfigureprocessor;


@TestConfiguration
@TestConditionalOnClass(name = "java.io.InputStream", value = TestClassConfiguration.Nested.class)
public class TestClassConfiguration {

	@TestAutoConfigureOrder
	public static class Nested {

	}

}
