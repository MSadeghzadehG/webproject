

package org.springframework.boot.test.json;

import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.InputStream;

import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.rules.TemporaryFolder;

import org.springframework.core.io.ByteArrayResource;
import org.springframework.core.io.Resource;
import org.springframework.util.FileCopyUtils;

import static org.assertj.core.api.Assertions.assertThat;


public class BasicJsonTesterTests {

	private static final String JSON = "{\"spring\":[\"boot\",\"framework\"]}";

	@Rule
	public ExpectedException thrown = ExpectedException.none();

	@Rule
	public TemporaryFolder tempFolder = new TemporaryFolder();

	private BasicJsonTester json = new BasicJsonTester(getClass());

	@Test
	public void createWhenResourceLoadClassIsNullShouldThrowException() {
		this.thrown.expect(IllegalArgumentException.class);
		this.thrown.expectMessage("ResourceLoadClass must not be null");
		new BasicJsonTester(null);
	}

	@Test
	public void fromJsonStringShouldReturnJsonContent() {
		assertThat(this.json.from(JSON)).isEqualToJson("source.json");
	}

	@Test
	public void fromResourceStringShouldReturnJsonContent() {
		assertThat(this.json.from("source.json")).isEqualToJson(JSON);
	}

	@Test
	public void fromResourceStringWithClassShouldReturnJsonContent() {
		assertThat(this.json.from("source.json", getClass())).isEqualToJson(JSON);
	}

	@Test
	public void fromByteArrayShouldReturnJsonContent() {
		assertThat(this.json.from(JSON.getBytes())).isEqualToJson("source.json");
	}

	@Test
	public void fromFileShouldReturnJsonContent() throws Exception {
		File file = this.tempFolder.newFile("file.json");
		FileCopyUtils.copy(JSON.getBytes(), file);
		assertThat(this.json.from(file)).isEqualToJson("source.json");
	}

	@Test
	public void fromInputStreamShouldReturnJsonContent() {
		InputStream inputStream = new ByteArrayInputStream(JSON.getBytes());
		assertThat(this.json.from(inputStream)).isEqualToJson("source.json");
	}

	@Test
	public void fromResourceShouldReturnJsonContent() {
		Resource resource = new ByteArrayResource(JSON.getBytes());
		assertThat(this.json.from(resource)).isEqualToJson("source.json");
	}

}
