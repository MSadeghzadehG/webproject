

package sample.undertow;

import java.io.ByteArrayInputStream;
import java.nio.charset.StandardCharsets;
import java.util.zip.GZIPInputStream;

import org.junit.Test;
import org.junit.runner.RunWith;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.context.SpringBootTest.WebEnvironment;
import org.springframework.boot.test.web.client.TestRestTemplate;
import org.springframework.http.HttpEntity;
import org.springframework.http.HttpHeaders;
import org.springframework.http.HttpMethod;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.test.context.junit4.SpringRunner;
import org.springframework.util.StreamUtils;

import static org.assertj.core.api.Assertions.assertThat;


@RunWith(SpringRunner.class)
@SpringBootTest(webEnvironment = WebEnvironment.RANDOM_PORT)
public class SampleUndertowApplicationTests {

	@Autowired
	private TestRestTemplate restTemplate;

	@Test
	public void testHome() {
		assertOkResponse("/", "Hello World");
	}

	@Test
	public void testAsync() {
		assertOkResponse("/async", "async: Hello World");
	}

	@Test
	public void testCompression() throws Exception {
		HttpHeaders requestHeaders = new HttpHeaders();
		requestHeaders.set("Accept-Encoding", "gzip");
		HttpEntity<?> requestEntity = new HttpEntity<>(requestHeaders);
		ResponseEntity<byte[]> entity = this.restTemplate.exchange("/", HttpMethod.GET,
				requestEntity, byte[].class);
		assertThat(entity.getStatusCode()).isEqualTo(HttpStatus.OK);

		try (GZIPInputStream inflater = new GZIPInputStream(
				new ByteArrayInputStream(entity.getBody()))) {
			assertThat(StreamUtils.copyToString(inflater, StandardCharsets.UTF_8))
					.isEqualTo("Hello World");
		}
	}

	private void assertOkResponse(String path, String body) {
		ResponseEntity<String> entity = this.restTemplate.getForEntity(path,
				String.class);
		assertThat(entity.getStatusCode()).isEqualTo(HttpStatus.OK);
		assertThat(entity.getBody()).isEqualTo(body);
	}

}
