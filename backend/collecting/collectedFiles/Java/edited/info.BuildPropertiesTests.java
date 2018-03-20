

package org.springframework.boot.info;

import java.time.Instant;
import java.time.format.DateTimeFormatter;
import java.time.temporal.ChronoUnit;
import java.util.Properties;

import org.junit.Test;

import static org.assertj.core.api.Assertions.assertThat;


public class BuildPropertiesTests {

	@Test
	public void basicInfo() {
		Instant instant = Instant.now();
		BuildProperties properties = new BuildProperties(createProperties("com.example",
				"demo", "0.0.1", DateTimeFormatter.ISO_INSTANT.format(instant)));
		assertThat(properties.getGroup()).isEqualTo("com.example");
		assertThat(properties.getArtifact()).isEqualTo("demo");
		assertThat(properties.getVersion()).isEqualTo("0.0.1");
		assertThat(properties.getTime())
				.isEqualTo(instant.truncatedTo(ChronoUnit.MILLIS));
		assertThat(properties.get("time"))
				.isEqualTo(String.valueOf(instant.toEpochMilli()));
	}

	@Test
	public void noInfo() {
		BuildProperties properties = new BuildProperties(new Properties());
		assertThat(properties.getGroup()).isNull();
		assertThat(properties.getArtifact()).isNull();
		assertThat(properties.getVersion()).isNull();
		assertThat(properties.getTime()).isNull();
	}

	private static Properties createProperties(String group, String artifact,
			String version, String buildTime) {
		Properties properties = new Properties();
		properties.put("group", group);
		properties.put("artifact", artifact);
		properties.put("version", version);
		properties.put("time", buildTime);
		return properties;
	}

}
