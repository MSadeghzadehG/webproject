

package org.springframework.boot.actuate.info;

import java.time.Instant;
import java.util.Map;
import java.util.Properties;

import org.springframework.boot.info.GitProperties;
import org.springframework.core.env.PropertiesPropertySource;
import org.springframework.core.env.PropertySource;


public class GitInfoContributor extends InfoPropertiesInfoContributor<GitProperties> {

	public GitInfoContributor(GitProperties properties) {
		this(properties, Mode.SIMPLE);
	}

	public GitInfoContributor(GitProperties properties, Mode mode) {
		super(properties, mode);
	}

	@Override
	public void contribute(Info.Builder builder) {
		builder.withDetail("git", generateContent());
	}

	@Override
	protected PropertySource<?> toSimplePropertySource() {
		Properties props = new Properties();
		copyIfSet(props, "branch");
		String commitId = getProperties().getShortCommitId();
		if (commitId != null) {
			props.put("commit.id", commitId);
		}
		copyIfSet(props, "commit.time");
		return new PropertiesPropertySource("git", props);
	}

	
	@Override
	protected void postProcessContent(Map<String, Object> content) {
		replaceValue(getNestedMap(content, "commit"), "time",
				getProperties().getCommitTime());
		replaceValue(getNestedMap(content, "build"), "time",
				getProperties().getInstant("build.time"));
	}

}
