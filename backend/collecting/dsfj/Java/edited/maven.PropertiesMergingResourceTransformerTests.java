

package org.springframework.boot.maven;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.util.jar.JarOutputStream;

import org.junit.Test;

import static org.assertj.core.api.Assertions.assertThat;


public class PropertiesMergingResourceTransformerTests {

	private final PropertiesMergingResourceTransformer transformer = new PropertiesMergingResourceTransformer();

	@Test
	public void testProcess() throws Exception {
		assertThat(this.transformer.hasTransformedResource()).isFalse();
		this.transformer.processResource("foo",
				new ByteArrayInputStream("foo=bar".getBytes()), null);
		assertThat(this.transformer.hasTransformedResource()).isTrue();
	}

	@Test
	public void testMerge() throws Exception {
		this.transformer.processResource("foo",
				new ByteArrayInputStream("foo=bar".getBytes()), null);
		this.transformer.processResource("bar",
				new ByteArrayInputStream("foo=spam".getBytes()), null);
		assertThat(this.transformer.getData().getProperty("foo")).isEqualTo("bar,spam");
	}

	@Test
	public void testOutput() throws Exception {
		this.transformer.setResource("foo");
		this.transformer.processResource("foo",
				new ByteArrayInputStream("foo=bar".getBytes()), null);
		ByteArrayOutputStream out = new ByteArrayOutputStream();
		JarOutputStream os = new JarOutputStream(out);
		this.transformer.modifyOutputStream(os);
		os.flush();
		os.close();
		assertThat(out.toByteArray()).isNotNull();
		assertThat(out.toByteArray().length > 0).isTrue();
	}

}
