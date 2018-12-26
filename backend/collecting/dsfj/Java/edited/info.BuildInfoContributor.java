

package org.springframework.boot.actuate.info;

import java.util.Map;
import java.util.Properties;

import org.springframework.boot.info.BuildProperties;
import org.springframework.core.env.PropertiesPropertySource;
import org.springframework.core.env.PropertySource;


public class BuildInfoContributor extends InfoPropertiesInfoContributor<BuildProperties> {

	public BuildInfoContributor(BuildProperties properties) {
		super(properties, Mode.FULL);
	}

	@Override
	public void contribute(Info.Builder builder) {
		builder.withDetail("build", generateContent());
	}

	@Override
	protected PropertySource<?> toSimplePropertySource() {
		Properties props = new Properties();
		copyIfSet(props, "group");
		copyIfSet(props, "artifact");
		copyIfSet(props, "name");
		copyIfSet(props, "version");
		copyIfSet(props, "time");
		return new PropertiesPropertySource("build", props);
	}

	@Override
	protected void postProcessContent(Map<String, Object> content) {
		replaceValue(content, "time", getProperties().getTime());
	}

}
