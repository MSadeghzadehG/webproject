

package org.springframework.boot.configurationprocessor.metadata;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.util.Arrays;
import java.util.Collections;

import org.junit.Test;

import static org.assertj.core.api.Assertions.assertThat;


public class JsonMarshallerTests {

	@Test
	public void marshallAndUnmarshal() throws Exception {
		ConfigurationMetadata metadata = new ConfigurationMetadata();
		metadata.add(ItemMetadata.newProperty("a", "b", StringBuffer.class.getName(),
				InputStream.class.getName(), "sourceMethod", "desc", "x",
				new ItemDeprecation("Deprecation comment", "b.c.d")));
		metadata.add(ItemMetadata.newProperty("b.c.d", null, null, null, null, null, null,
				null));
		metadata.add(
				ItemMetadata.newProperty("c", null, null, null, null, null, 123, null));
		metadata.add(
				ItemMetadata.newProperty("d", null, null, null, null, null, true, null));
		metadata.add(ItemMetadata.newProperty("e", null, null, null, null, null,
				new String[] { "y", "n" }, null));
		metadata.add(ItemMetadata.newProperty("f", null, null, null, null, null,
				new Boolean[] { true, false }, null));
		metadata.add(ItemMetadata.newGroup("d", null, null, null));
		metadata.add(ItemHint.newHint("a.b"));
		metadata.add(ItemHint.newHint("c", new ItemHint.ValueHint(123, "hey"),
				new ItemHint.ValueHint(456, null)));
		metadata.add(new ItemHint("d", null,
				Arrays.asList(
						new ItemHint.ValueProvider("first",
								Collections.singletonMap("target", "foo")),
						new ItemHint.ValueProvider("second", null))));
		ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
		JsonMarshaller marshaller = new JsonMarshaller();
		marshaller.write(metadata, outputStream);
		ConfigurationMetadata read = marshaller
				.read(new ByteArrayInputStream(outputStream.toByteArray()));
		assertThat(read).has(Metadata.withProperty("a.b", StringBuffer.class)
				.fromSource(InputStream.class).withDescription("desc")
				.withDefaultValue("x").withDeprecation("Deprecation comment", "b.c.d"));
		assertThat(read).has(Metadata.withProperty("b.c.d"));
		assertThat(read).has(Metadata.withProperty("c").withDefaultValue(123));
		assertThat(read).has(Metadata.withProperty("d").withDefaultValue(true));
		assertThat(read).has(
				Metadata.withProperty("e").withDefaultValue(new String[] { "y", "n" }));
		assertThat(read).has(Metadata.withProperty("f")
				.withDefaultValue(new Object[] { true, false }));
		assertThat(read).has(Metadata.withGroup("d"));
		assertThat(read).has(Metadata.withHint("a.b"));
		assertThat(read).has(
				Metadata.withHint("c").withValue(0, 123, "hey").withValue(1, 456, null));
		assertThat(read).has(Metadata.withHint("d").withProvider("first", "target", "foo")
				.withProvider("second"));
	}

}
